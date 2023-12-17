#version 330 core

in vec3 fragWorldPos;
in vec3 fragWorldNormal;
out vec4 fragColor;

const int MAX_LIGHTS = 8;

struct Light {
    int type;
    vec4 position;
    vec4 direction;
    vec4 color;
    vec3 attenuation;
    float angle;
    float penumbra;
};

uniform Light lights[MAX_LIGHTS];
uniform int numLights;

uniform float k_a;
uniform vec4 cAmbient;

uniform float k_d;
uniform vec4 cDiffuse;

uniform float k_s;
uniform vec4 cSpecular;

uniform float shininess;
uniform vec4 cameraPos;

void main() {
    fragColor = cAmbient * k_a;

    for (int i = 0; i < numLights; i++) {
        Light light = lights[i];

        if (light.type == 0) { // LIGHT_POINT
            vec3 lightDir = normalize(light.position.xyz - fragWorldPos);
            float distance = distance(light.position.xyz, fragWorldPos);
            float attenuationFactor = 1.0 / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * distance * distance);
            attenuationFactor = clamp(attenuationFactor, 0.0, 1.0);
            light.color *= attenuationFactor;
            float diffuseIntensity = max(dot(normalize(fragWorldNormal), lightDir), 0.0);
            fragColor += cDiffuse * (k_d * diffuseIntensity) * light.color;
            vec3 viewDir = normalize(cameraPos.xyz - fragWorldPos);
            vec3 reflectDir = reflect(-lightDir, normalize(fragWorldNormal));
            float specularIntensity;
            if (shininess != 0) {
                specularIntensity = pow(max(0.0, dot(reflectDir, viewDir)), shininess);
            } else {
                specularIntensity = 1.0;
            }
            fragColor += cSpecular * (k_s * specularIntensity) * light.color;
        } else if (light.type == 1) { // LIGHT_DIRECTIONAL
            vec3 lightDir = normalize(-light.direction.xyz);
            float diffuseIntensity = max(dot(normalize(fragWorldNormal), lightDir), 0.0);
            fragColor += cDiffuse * (k_d * diffuseIntensity) * light.color;
            vec3 viewDir = normalize(cameraPos.xyz - fragWorldPos);
            vec3 reflectDir = reflect(-lightDir, normalize(fragWorldNormal));
            float specularIntensity;
            if (shininess != 0) {
                specularIntensity = pow(max(0.0, dot(reflectDir, viewDir)), shininess);
            } else {
                specularIntensity = 1.0;
            }
            fragColor += cSpecular * (k_s * specularIntensity) * light.color;
        } else if (light.type == 2) { // LIGHT_SPOT
            vec3 lightDir = normalize(light.position.xyz - fragWorldPos);

            float cosTheta = dot(-lightDir, normalize(light.direction.xyz));
            float thetaOuter = light.angle;
            float penumbra = light.penumbra;
            float thetaInner = thetaOuter - penumbra;

            float x = acos(cosTheta);

            if (x <= thetaInner) {
                // Inside the inner cone, full intensity
                // Perform calculations with full intensity
                float distance = distance(light.position.xyz, fragWorldPos);
                float attenuation = min(1.0f, 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * distance * distance));
                light.color *= attenuation;

                float diffuseIntensity = max(dot(normalize(fragWorldNormal), lightDir), 0.0);
                fragColor += cDiffuse * (k_d * diffuseIntensity) * light.color;

                vec3 viewDir = normalize(cameraPos.xyz - fragWorldPos);
                vec3 reflectDir = reflect(-lightDir, normalize(fragWorldNormal));
                float specularIntensity;
                if (shininess != 0) {
                    specularIntensity = pow(max(0.0, dot(reflectDir, viewDir)), shininess);
                } else {
                    specularIntensity = 1.0;
                }
                fragColor += cSpecular * (k_s * specularIntensity) * light.color;
            } else if (thetaInner < x && x <= thetaOuter) {
                // Inside the outer cone but outside the inner cone, interpolate intensity
                float smoothstepFactor = smoothstep(thetaOuter, thetaInner, x);

                // Perform calculations with interpolated intensity
                float distance = distance(light.position.xyz, fragWorldPos);
                float attenuation = min(1.0f, 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * distance * distance));
                light.color *= attenuation;

                float diffuseIntensity = max(dot(normalize(fragWorldNormal), lightDir), 0.0);
                fragColor += cDiffuse * (k_d * diffuseIntensity * smoothstepFactor) * light.color;

                vec3 viewDir = normalize(cameraPos.xyz - fragWorldPos);
                vec3 reflectDir = reflect(-lightDir, normalize(fragWorldNormal));
                float specularIntensity;
                if (shininess != 0) {
                    specularIntensity = pow(max(0.0, dot(reflectDir, viewDir)), shininess);
                } else {
                    specularIntensity = 1.0;
                }
                fragColor += cSpecular * (k_s * specularIntensity * smoothstepFactor) * light.color;
            }
        }

    }
}
