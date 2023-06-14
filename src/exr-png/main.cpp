#include <OpenEXR/ImfRgbaFile.h>
#include <lodepng.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <limits>

#define clamp(v, l, u) std::min(std::max(v, l), u)


inline double to_sRGB(double rgb_color)
{
    const double a = 0.055;

    if (rgb_color <= 0.0031308) {
        return 12.92 * rgb_color;
	} else {
        return (1.0 + a) * std::pow(rgb_color, 1.0 / 2.4) - a;
	}
}


inline double gamma_correction(double rgb_color, double gamma)
{
	return std::pow(rgb_color, 1.0/gamma);
}


int main(int argc, char *argv[])
{
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
	TCLAP::ValueArg<double> gammaCorrection
		("g", "gamma",
		 "Set the gamma correction (default sRGB)",
		 false, 1.0, "real");
	TCLAP::ValueArg<double> exposure
		("e", "exposure",
		 "Set the exposure",
		 false, 0.0, "real");

    cmd.add(inputFile);
    cmd.add(outputFile);
	cmd.add(ignoreAlpha);
	cmd.add(gammaCorrection);
    cmd.add(exposure);
	cmd.parse(argc, argv);

    // Load the OpenEXR file
    Imf::RgbaInputFile file(inputFile.getValue().c_str());
    Imath::Box2i dw = file.dataWindow();
    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

	std::vector<Imf::Rgba> pixels_exr(width * height);
    file.setFrameBuffer(&pixels_exr[0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);

    // Create the data for PNG output
    std::vector<unsigned char> pixels_png(width * height * 4);

    double exposureMul = pow(2.0, exposure.getValue());

	if (gammaCorrection.isSet()) {
		double gammaValue = gammaCorrection.getValue();
		// Transform colors to 8bit sRGB
		for (int i = 0; i < width * height; i++) {
			pixels_png[4 * i + 0] =
				(unsigned char)(gamma_correction(clamp((double)(pixels_exr[i].r) * exposureMul, 0.0, 1.0), gammaValue) * (float)std::numeric_limits<uint8_t>::max());
			pixels_png[4 * i + 1] =
				(unsigned char)(gamma_correction(clamp((double)(pixels_exr[i].g) * exposureMul, 0.0, 1.0), gammaValue) * (float)std::numeric_limits<uint8_t>::max());
			pixels_png[4 * i + 2] =
				(unsigned char)(gamma_correction(clamp((double)(pixels_exr[i].b) * exposureMul, 0.0, 1.0), gammaValue) * (float)std::numeric_limits<uint8_t>::max());
			if (ignoreAlpha.getValue()) {
				pixels_png[4 * i + 3] = 0xFF;
			} else {
				pixels_png[4 * i + 3] =
					(unsigned char)(clamp((double)(pixels_exr[i].a), 0.0, 1.0) * (float)std::numeric_limits<uint8_t>::max());
			}
		}
	} else {
		// Transform colors to 8bit sRGB
		for (int i = 0; i < width * height; i++) {
			pixels_png[4 * i + 0] =
				(unsigned char)(to_sRGB(clamp((double)(pixels_exr[i].r) * exposureMul, 0.0, 1.0)) * (float)std::numeric_limits<uint8_t>::max());
			pixels_png[4 * i + 1] =
				(unsigned char)(to_sRGB(clamp((double)(pixels_exr[i].g) * exposureMul, 0.0, 1.0)) * (float)std::numeric_limits<uint8_t>::max());
			pixels_png[4 * i + 2] =
				(unsigned char)(to_sRGB(clamp((double)(pixels_exr[i].b) * exposureMul, 0.0, 1.0)) * (float)std::numeric_limits<uint8_t>::max());
			if (ignoreAlpha.getValue()) {
				pixels_png[4 * i + 3] = 0xFF;
			} else {
				pixels_png[4 * i + 3] =
					(unsigned char)(clamp((double)(pixels_exr[i].a), 0.0, 1.0) * (float)std::numeric_limits<uint8_t>::max());
			}
		}
	}

	unsigned error = lodepng::encode(outputFile.getValue(), pixels_png, width, height);
	if (error)
		std::cout << "encoder error " << error
				  << ": "<< lodepng_error_text(error) << std::endl;

    return 0;
}
