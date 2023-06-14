#include <iostream>
#include <cstdlib>
#include <cstring>
#include <tiffio.h>

#include <OpenEXR/ImfRgbaFile.h>

void writeRgba(const char filename[], const Imf::Rgba* pixels, int width, int height) {
    Imf::RgbaOutputFile file(filename, width, height, Imf::WRITE_RGBA);
    file.setFrameBuffer(pixels, 1, width);
    file.writePixels(height);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage : " << std::endl
                  << argv[0] << " <input_tiff_file> <output_exr_file>" << std::endl;
        exit(1);
    }

    std::cout << "Reading: " << argv[1] << "..." << std::endl;

    TIFF* tif = TIFFOpen(argv[1], "r");

    if (tif) {
        uint32 w, h;
        uint16 bps, spp;
        uint16 config;

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
        memset(image, 0xFF, w*h*4);

        switch(config) {
        case PLANARCONFIG_CONTIG:
            switch(bps) {
            case 8:
                for (uint32 row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint8 *scanline = (uint8*)buf;
                    for (uint32 col = 0; col < w; col++) {
                        half *channels = (half*) (image  + (row * w + col));
                        channels[3] = 1.0;
                        if (spp >= 3) {
                            for (int c = 0; c < spp; c++) {
                                // Write to EXR
                                // TODO: Convert sRGB to Linear RGB
                                channels[c] = (half)scanline[spp * col + c] / 256.0f;
                            }
                        } else {
                            for (int c = 0; c < 3; c++) {
                                // Write to EXR
                                // TODO: Convert sRGB to Linear RGB
                                channels[c] = (half)scanline[spp * col] / 256.0f;
                            }
                        }
                    }
                }
                break;
            case 16:
                for (uint32 row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint16 *scanline = (uint16*)buf;
                    for (uint32 col = 0; col < w; col++) {
                        half *channels = (half*) (image + (row * w + col));
                        channels[3] = 1.0;
                        if (spp >= 3) {
                            for (int c = 0; c < spp; c++) {
                                // Write to EXR
                                channels[c] = (half)scanline[spp * col + c] / 65536.0f;
                            }
                        } else {
                            for (int c = 0; c < 3; c++) {
                                // Write to EXR
                                channels[c] = (half)scanline[spp * col] / 65536.0f;
                            }
                        }
                    }
                }
                break;
            case 32:
                for (uint32 row = 0; row < h; row++) {
                    TIFFReadScanline(tif, buf, row, 0);
                    uint32 *scanline = (uint32*)buf;
                    for (uint32 col = 0; col < w; col++) {
                        half *channels = (half*) (image + (row * w + col));
                        channels[3] = 1.0;
                        for (int c = 0; c < spp; c++) {
                            // Write to EXR
                            channels[c] = (half)scanline[spp * col + c] / 4294967296.0f;
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
                    for (uint32 row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint8 *scanline = (uint8*)buf;
                        for (uint32 col = 0; col < w; col++) {
                            // Write to PNG
                            //image[4 * (row * w + col) + c] = scanline[spp * col + c];
                        }
                    }
                }
                break;
            case 16:
                for (int c = 0; c < spp; c++) {
                    for (uint32 row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint8 *scanline = (uint8*)buf;
                        for (uint32 col = 0; col < w; col++) {
                            // Write to PNG
                            //image[4 * (row * w + col) + c] = scanline[spp * col + c] / 65535.0f;
                        }
                    }
                }
                break;
            case 32:
                for (int c = 0; c < spp; c++) {
                    for (uint32 row = 0; row < h; row++) {
                        TIFFReadScanline(tif, buf, row, c);
                        uint8 *scanline = (uint8*)buf;
                        for (uint32 col = 0; col < w; col++) {
                            // Write to PNG
                            //image[4 * (row * w + col) + c] = scanline[spp * col + c] / 16777216;;
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
