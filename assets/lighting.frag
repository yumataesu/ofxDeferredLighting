#version 410

const int Max_Light_Num = 24;
struct Light {
    vec4 diffuse;
    vec3 position;
};

uniform sampler2D color_tex;
uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D ssao;
uniform mat4 view;
uniform int u_light_num;
uniform vec3 u_campos;
uniform vec3 u_light_positions[Max_Light_Num];
uniform Light lights[Max_Light_Num];

in vec2 v_texcoord;

out vec4 fragColor;

void main() {
    vec3 world = texture(position_tex, v_texcoord).rgb;
    vec3 color = texture(color_tex, v_texcoord).xyz;
    vec3 normal = texture(normal_tex, v_texcoord).xyz;
    vec3 ao = texture(ssao, v_texcoord).rgb;

    float ul = u_light_num;
    vec4 result = vec4(0);
    vec3 d = vec3(0);
    for(int i = 0; i < Max_Light_Num; ++i) {
        if(u_light_num - 1 < i) break;


        //Diffuse
        vec3 v_light_pos = (view * vec4(lights[i].position, 1.0)).xyz;
        vec3 lightDir = normalize(v_light_pos - world);
        vec3 diffuse = max(dot(normal, lightDir), 0.0) * lights[i].diffuse.rgb;

        //Specular
        vec3 viewDir  = normalize(u_campos - world);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = vec3(1.0) * spec;
        //d += viewDir;
            
        //Attenuation
        float dist = length(v_light_pos - world) * 0.001;
        float attenuation = 1.0 + (0.001 * dist) + (0.01 * dist * dist); // magic number

        result += vec4(diffuse + specular, 1.0);
    }

    color = pow(color.rgb, vec3(2.2));
    fragColor = vec4(color * vec3(result) * ao.r, 1.0);
    fragColor = vec4(vec3(ao.r), 1.0);

}
