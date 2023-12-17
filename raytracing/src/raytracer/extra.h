/*
Anti-aliasing (up to 3 points): If you only shoot one ray through the center of each pixel, you'll see jaggies (aliasing)
due to undersampling of high-frequency parts of the scene (e.g. object edges). In the Filter unit, we discussed some ways to
ameliorate (but not completely fix) aliasing with some post-processing. See if you can integrate your Filter code into your ray
tracer as a post-processing module for your output image.

Please create a helper method that antialiases (keep it simple: use simplest blur filter)
 */
// Extra credit helper method for antialiasing (3 points)
void applySimpleBlurFilter(RGBA* imageData, int width, int height, int numPasses) {
    // Define a simple 3x3 blur filter kernel
    float blurKernel[9] = {
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
        1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f
    };
    // Create a temporary buffer to store the blurred image
    std::vector<RGBA> blurredImage(width * height);
    for (int pass = 0; pass < numPasses; pass++) {
        // Apply the blur filter to each pixel in the image
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                float r = 0.0f, g = 0.0f, b = 0.0f;
                int index = x + y * width;
                for (int j = -1; j <= 1; j++) {
                    for (int i = -1; i <= 1; i++) {
                        int neighborIndex = (x + i) + (y + j) * width;
                        r += imageData[neighborIndex].r * blurKernel[(j + 1) * 3 + (i + 1)];
                        g += imageData[neighborIndex].g * blurKernel[(j + 1) * 3 + (i + 1)];
                        b += imageData[neighborIndex].b * blurKernel[(j + 1) * 3 + (i + 1)];
                    }
                }
                blurredImage[index] = RGBA{static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
            }
        }
        // Copy the blurred image back to the original imageData for the next pass
        for (int i = 0; i < width * height; i++) {
            imageData[i] = blurredImage[i];
        }
    }
}
