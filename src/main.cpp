#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <lodepng.h>
#include <tclap/CmdLine.h>
#include <iostream>

#define clamp(v, l, u) std::min(std::max(v, l), u)

inline double to_sRGB(double rgb_color) {
    const double a = 0.055;
    if (rgb_color <= 0.0031308)
        return 12.92 * rgb_color;
    else
        return (1.0 + a) * std::pow(rgb_color, 1.0 / 2.4) - a;
}

int main(int argc, char *argv[]) {
    TCLAP::CmdLine cmd
		("Utility for converting OpenEXR file to PNG",
		 ' ',
		 "0.1");
    TCLAP::ValueArg<std::string> inputFile
		("i", "input",
		 "Set file to convert",
		 true, "", "string");
    TCLAP::ValueArg<std::string> outputFile
		("o", "output",
		 "Set file to save to",
		 true, "", "string");
    TCLAP::SwitchArg ignoreAlpha
		("a", "ignore-alpha",
		 "Ignore alpha channel",
		 false);
	TCLAP::ValueArg<double> gammaCorreciton
		("g", "gamma",
		 "Set the gamma correction (default sRGB)",
		 false, 1.0, "real");
	
    cmd.add(inputFile);
    cmd.add(outputFile);
	cmd.add(ignoreAlpha);
	//cmd.add(gammaCorreciton);
	cmd.parse(argc, argv);
	
    // Load the OpenEXR file
    Imf::Array2D<Imf::Rgba> pixels;
    Imf::RgbaInputFile file(inputFile.getValue().c_str());
    Imath::Box2i dw = file.dataWindow();
    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

    pixels.resizeErase(height, width);
    file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);

    // Create the data for PNG output
    std::vector<unsigned char> image;
    image.resize(width * height * 4);

    // Transform colors to 8bit sRGB
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
			image[4 * width * y + 4 * x + 0] =
				(unsigned char)(to_sRGB(clamp((double)(pixels[y][x].r), 0.0, 1.0)) * 255.0);
			image[4 * width * y + 4 * x + 1] =
				(unsigned char)(to_sRGB(clamp((double)(pixels[y][x].g), 0.0, 1.0)) * 255.0);
			image[4 * width * y + 4 * x + 2] =
				(unsigned char)(to_sRGB(clamp((double)(pixels[y][x].b), 0.0, 1.0)) * 255.0);
			if (ignoreAlpha.getValue()) {
				image[4 * width * y + 4 * x + 3] = 0xFF;
			} else {
				image[4 * width * y + 4 * x + 0] =
				(unsigned char)(to_sRGB(clamp((double)(pixels[y][x].a), 0.0, 1.0)) * 255.0);
			}
        }
    }

    unsigned error = lodepng::encode(outputFile.getValue(), image, width, height);

	if (error)
		std::cout << "encoder error " << error
				  << ": "<< lodepng_error_text(error) << std::endl;
	
    return 0;
}
