#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <tiffio.h>

#include <OpenEXR/ImfRgbaFile.h>


inline double to_linear_RGB(double rgb_color)
{
    const double a = 0.055;

    if (rgb_color <= 0.04045) {
        return rgb_color / 12.92;
    } else {
        return std::pow((rgb_color + a) / (1.0 + a), 2.4);
    }
}


void writeRgba(const char filename[], const Imf::Rgba* pixels, int width, int height)
{
    Imf::RgbaOutputFile file(filename, width, height, Imf::WRITE_RGBA);
    file.setFrameBuffer(pixels, 1, width);
    file.writePixels(height);
}


int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cout << "Usage : " << std::endl
                  << argv[0] << " <input_tiff_file> <output_exr_file>" << std::endl;
        exit(1);
    }

    std::cout << "Reading: " << argv[1] << "..." << std::endl;

    TIFF* tif = TIFFOpen(argv[1], "r");

    if (tif) {
        uint32_t w, h;
        uint16_t bps, spp;
        uint16_t config;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

        std::cout << "Resolution: " << w << "px x " <<  h << "px" << std::endl
                  << "Bits per sample: " << bps << "bits" << std::endl
                  << "Samples per pixel: " << spp << std::endl;

        tdata_t buf;

        buf = _TIFFmalloc(TIFFScanlineSize(tif));

        // OpenEXR out image
        Imf::Rgba* image = new Imf::Rgba[w*h];

        // Fill with 255 for non alpha
        memset((char*)image, 0xFF, w * h * sizeof(Imf::Rgba));

        switch(config) {
        case PLANARCONFIG_CONTIG:
            switch(bps) {
            case 8:
                for (uint32_t row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint8_t *scanline = (uint8_t*)buf;
                    for (uint32_t col = 0; col < w; col++) {
                        half *channels = (half*) (image  + (row * w + col));
                        channels[3] = 1.0;
                        if (spp >= 3) {
                            for (int c = 0; c < spp; c++) {
                                // Write to EXR
                                channels[c] = to_linear_RGB((half)scanline[spp * col + c] / (half)std::numeric_limits<uint8_t>::max());
                            }
                        } else {
                            for (int c = 0; c < 3; c++) {
                                // Write to EXR
                                channels[c] = to_linear_RGB((half)scanline[spp * col] / (half)std::numeric_limits<uint8_t>::max());
                            }
                        }
                    }
                }
                break;
            case 16:
                for (uint32_t row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint16_t *scanline = (uint16_t*)buf;
                    for (uint32_t col = 0; col < w; col++) {
                        half *channels = (half*) (image + (row * w + col));
                        channels[3] = 1.0;
                        if (spp >= 3) {
                            for (int c = 0; c < spp; c++) {
                                // Write to EXR
                                channels[c] = (half)scanline[spp * col + c] / (half)std::numeric_limits<uint16_t>::max();
                            }
                        } else {
                            for (int c = 0; c < 3; c++) {
                                // Write to EXR
                                channels[c] = (half)scanline[spp * col] / (half)std::numeric_limits<uint16_t>::max();
                            }
                        }
                    }
                }
                break;
            case 32:
                for (uint32_t row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint32_t *scanline = (uint32_t*)buf;
                    for (uint32_t col = 0; col < w; col++) {
                        half *channels = (half*) (image + (row * w + col));
                        channels[3] = 1.0;
                        for (int c = 0; c < spp; c++) {
                            // Write to EXR
                            channels[c] = (half)scanline[spp * col + c] / (half)std::numeric_limits<uint32_t>::max();
                        }
                    }
                }
                break;
            default:
                std::cerr << "Not Supported" << std::endl;
                break;
            }
            break;
#ifdef TODO
        case PLANARCONFIG_SEPARATE:
            // TODO
            switch(bps) {
            case 8:
                for (int c = 0; c < spp; c++) {
                    for (uint32_t row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint8_t *scanline = (uint8_t*)buf;
                        for (uint32_t col = 0; col < w; col++) {
                            //image[4 * (row * w + col) + c] = (half)scanline[spp * col + c] / (half)std::numeric_limits<uint8_t>::max();
                        }
                    }
                }
                break;
            case 16:
                for (int c = 0; c < spp; c++) {
                    for (uint32_t row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint16_t *scanline = (uint16_t*)buf;
                        for (uint32_t col = 0; col < w; col++) {
                            //image[4 * (row * w + col) + c] = (half)scanline[spp * col + c] / (half)std::numeric_limits<uint16_t>::max();
                        }
                    }
                }
                break;
            case 32:
                for (int c = 0; c < spp; c++) {
                    for (uint32_t row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint32_t *scanline = (uint32_t*)buf;
                        for (uint32_t col = 0; col < w; col++) {
                            //image[4 * (row * w + col) + c] = (half)scanline[spp * col + c] / (half)std::numeric_limits<uint32_t>::max();
                        }
                    }
                }
            default:
                std::cerr << "Not Supported" << std::endl;
                break;
            }
            break;
#endif
        default:
            std::cerr << "Not Supported" << std::endl;
            break;
        }

        std::cout << "Writing: " << argv[2] << "..." << std::endl;

        writeRgba(argv[2], image, w, h);

        delete[] image;
        _TIFFfree(buf);
        TIFFClose(tif);
    }

    return EXIT_SUCCESS;
}
