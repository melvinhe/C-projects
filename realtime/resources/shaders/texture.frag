#version 330 core

in vec2 fragUVCoord;

uniform sampler2D ourTexture;
uniform float sceneWidth;
uniform float sceneHeight;
uniform bool invertFilter;
uniform bool sharpenFilter;
uniform bool grayscaleFilter;
uniform bool boxFilter;
uniform bool sepiaHueFilter;
uniform bool edgeFilter;

out vec4 fragColor;

void main() {
    vec4 currColor = texture(ourTexture, fragUVCoord);
    if (sharpenFilter) {
        mat3 sharpenKernel = mat3(-1, -1, -1, -1, 17, -1, -1, -1, -1) / 9.0;
        vec3 sum = vec3(0);
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                vec2 neighborCoord = fragUVCoord + vec2(float(i) / sceneWidth, float(j) / sceneHeight);
                vec3 neighborColor = texture(ourTexture, neighborCoord).rgb;
                sum += neighborColor * sharpenKernel[i + 1][j + 1];
            }
        }
        currColor.rgb = sum;
    }

    if (invertFilter) {
        currColor.rgb = 1.0 - currColor.rgb;
    }

    // Extra credit: 5x5 box filter (kernel-based/multi-stage), seperable edge filter (kernel-based/multi-stage)

    if (boxFilter) {
        vec3 sum = vec3(0); // using sum since mat5 undefined (same effect)
        for (int i = -2; i <= 2; ++i) {
            for (int j = -2; j <= 2; ++j) {
                vec2 neighborCoord = fragUVCoord + vec2(float(i) / sceneWidth, float(j) / sceneHeight);
                vec3 neighborColor = texture(ourTexture, neighborCoord).rgb;
                sum += neighborColor / 25.0;  // Divide by 25 for normalization
            }
        }
        currColor.rgb = sum;
    }

    if (edgeFilter) {
        vec3 sobelHorizontalKernelX = vec3(-1.0, 0.0, 1.0);
        vec3 sobelVerticalKernelX = vec3(1.0, 2.0, 1.0);
        vec3 sobelHorizontalKernelY = vec3(1.0, 2.0, 1.0);
        vec3 sobelVerticalKernelY = vec3(-1.0, 0.0, 1.0);

        float horizontalSum = 0.0;
        float verticalSum = 0.0;

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                vec2 neighborCoord = fragUVCoord + vec2(float(i) / sceneWidth, float(j) / sceneHeight);
                vec3 neighborColor = texture(ourTexture, neighborCoord).rgb;
                float grayscale = dot(neighborColor, vec3(0.2126, 0.7152, 0.0722));

                horizontalSum += grayscale * sobelHorizontalKernelX[i + 1] * sobelVerticalKernelX[j + 1];
                verticalSum += grayscale * sobelHorizontalKernelY[i + 1] * sobelVerticalKernelY[j + 1];
            }
        }

        float edgeMagnitude = length(vec2(horizontalSum, verticalSum));
        currColor = vec4(vec3(edgeMagnitude), 1.0);
    }

    // Extra credit: sepia hue (per-pixel), grayscale (per-pixel)

    if (sepiaHueFilter) {
        // Apply sepia hue filter (makes look more vintage)
        float r = dot(currColor.rgb, vec3(0.393, 0.769, 0.189));
        float g = dot(currColor.rgb, vec3(0.349, 0.686, 0.168));
        float b = dot(currColor.rgb, vec3(0.272, 0.534, 0.131));
        currColor.rgb = vec3(r, g, b);
    }

    if (grayscaleFilter) {
        float grayscale = dot(currColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        currColor = vec4(grayscale, grayscale, grayscale, currColor.a);
    }
    fragColor = currColor;

}
