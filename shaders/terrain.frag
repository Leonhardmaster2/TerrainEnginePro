#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform int renderMode; // 0=Solid, 1=Wireframe, 2=Clay, 3=Unlit, 4=Normals

void main() {
    vec3 normal = normalize(Normal);

    // Normals mode
    if (renderMode == 4) {
        FragColor = vec4(normal * 0.5 + 0.5, 1.0);
        return;
    }

    // Unlit mode
    if (renderMode == 3) {
        FragColor = vec4(Color, 1.0);
        return;
    }

    // Lighting calculation
    vec3 lightDirNorm = normalize(lightDir);

    // Ambient
    vec3 ambient = 0.3 * lightColor;

    // Diffuse
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.3 * spec * lightColor;

    vec3 baseColor;

    // Clay mode - uniform color with lighting
    if (renderMode == 2) {
        baseColor = vec3(0.8, 0.7, 0.6); // Clay color
    }
    // Solid mode - height-based coloring
    else {
        // Height-based terrain coloring
        float height = Color.r;

        vec3 waterColor = vec3(0.2, 0.3, 0.5);
        vec3 sandColor = vec3(0.8, 0.7, 0.5);
        vec3 grassColor = vec3(0.3, 0.6, 0.2);
        vec3 rockColor = vec3(0.5, 0.5, 0.5);
        vec3 snowColor = vec3(0.9, 0.9, 1.0);

        if (height < 0.2) {
            baseColor = mix(waterColor, sandColor, height / 0.2);
        } else if (height < 0.4) {
            baseColor = mix(sandColor, grassColor, (height - 0.2) / 0.2);
        } else if (height < 0.7) {
            baseColor = mix(grassColor, rockColor, (height - 0.4) / 0.3);
        } else {
            baseColor = mix(rockColor, snowColor, (height - 0.7) / 0.3);
        }
    }

    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
