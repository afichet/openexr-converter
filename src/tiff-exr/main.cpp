#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include <stdexcept>

#include <tiffio.h>

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfFrameBuffer.h>


void print_tif_info(
    uint32_t width, uint32_t height,
    uint16_t bps, uint16_t spp,
    uint16_t config,
    uint16_t type)
{
    std::cout << "  Resolution: " << width << "px x " <<  height << "px" << std::endl;
    std::cout << "  Type: ";

    switch (type) {
        case SAMPLEFORMAT_UINT:
            std::cout << "uint" << bps << std::endl;
            break;
        case SAMPLEFORMAT_INT:
            std::cout << "int" << bps << std::endl;
            break;
        case SAMPLEFORMAT_IEEEFP:
            std::cout << "float" << bps << std::endl;
            break;
        case SAMPLEFORMAT_VOID:
            std::cout << "void" << bps << std::endl;
            break;
        case SAMPLEFORMAT_COMPLEXINT:
            std::cout << "complex int" << bps << std::endl;
            break;
        case SAMPLEFORMAT_COMPLEXIEEEFP:
            std::cout << "complex float" << bps << std::endl;
            break;
        default:
            std::cout << "unrecognized" << std::endl;
            break;
    }

    std::cout << "  Planar config:";

    switch (config) {
        case PLANARCONFIG_CONTIG:
            std::cout << "contig" << std::endl;
            break;
        case PLANARCONFIG_SEPARATE:
            std::cout << "separate" << std::endl;
            break;
        default:
            std::cout << "unrecognized" << std::endl;
            break;
    }
}


inline float to_linear_RGB(float rgb_color)
{
    const float a = 0.055f;

    if (rgb_color <= 0.04045f) {
        return rgb_color / 12.92f;
    } else {
        return std::pow((rgb_color + a) / (1.0f + a), 2.4f);
    }
}


template <typename T>
void int_tif_to_float(
    uint32_t width, uint32_t height,
    uint16_t spp,
    uint16_t config,
    TIFF* tif,
    std::vector<float>& output_buffer)
{
    tdata_t buf;
    buf = _TIFFmalloc(TIFFScanlineSize(tif));

    output_buffer.resize(width * height * spp);

    switch (config) {
        case PLANARCONFIG_CONTIG:
            for (uint32_t y = 0; y < height; y++) {
                TIFFReadScanline(tif, buf, y, 0);
                T* scanline = (T*)buf;

                for (uint32_t x = 0; x < width; x++) {
                    for (uint16_t c = 0;  c < spp; c++) {
                        output_buffer[spp * (y * width + x) + c] = (float)scanline[spp * x + c] / (float)std::numeric_limits<T>::max();
                    }
                }
            }
            break;

        // TODO: This is untested code!
        case PLANARCONFIG_SEPARATE:
            for (uint16_t c = 0;  c < spp; c++) {
                for (uint32_t y = 0; y < height; y++) {
                    TIFFReadScanline(tif, buf, y, c);
                    T* scanline = (T*)buf;

                    for (uint32_t x = 0; x < width; x++) {
                        output_buffer[spp * (y * width + x) + c] = (float)scanline[x] / (float)std::numeric_limits<T>::max();
                    }
                }
            }
            break;

        default:
            _TIFFfree(buf);
            throw std::runtime_error("Invalid planar config");
    }

    _TIFFfree(buf);
}


template <typename T>
void float_tif_to_float(
    uint32_t width, uint32_t height,
    uint16_t spp,
    uint16_t config,
    TIFF* tif,
    std::vector<float>& output_buffer)
{
    tdata_t buf;
    buf = _TIFFmalloc(TIFFScanlineSize(tif));

    output_buffer.resize(width * height * spp);

    switch (config) {
        case PLANARCONFIG_CONTIG:
            for (uint32_t y = 0; y < height; y++) {
                TIFFReadScanline(tif, buf, y, 0);
                T* scanline = (T*)buf;

                for (uint32_t x = 0; x < width; x++) {
                    for (uint16_t c = 0;  c < spp; c++) {
                        output_buffer[spp * (y * width + x) + c] = (float)scanline[spp * x + c];
                    }
                }
            }
            break;

        // TODO: This is untested code!
        case PLANARCONFIG_SEPARATE:
            for (uint16_t c = 0;  c < spp; c++) {
                for (uint32_t y = 0; y < height; y++) {
                    TIFFReadScanline(tif, buf, y, c);
                    T* scanline = (T*)buf;

                    for (uint32_t x = 0; x < width; x++) {
                        output_buffer[spp * (y * width + x) + c] = (float)scanline[x];
                    }
                }
            }
            break;

        default:
            _TIFFfree(buf);
            throw std::runtime_error("Invalid planar config");

    }

    _TIFFfree(buf);
}


void write_exr(
    const std::string& filename,
    uint32_t width, uint32_t height,
    uint16_t spp,
    const std::vector<float>& buffer)
{
    Imf::Header exr_header(width, height);
    Imf::ChannelList &exr_channels = exr_header.channels();
    Imf::FrameBuffer exr_framebuffer;

    if (spp == 1) {
        Imf::Slice slice = Imf::Slice::Make(
            Imf::FLOAT,
            &buffer[0],
            exr_header.dataWindow(),
            sizeof(float), sizeof(float) * width
        );

        exr_channels.insert("Y", Imf::Channel(Imf::FLOAT));
        exr_framebuffer.insert("Y", slice);
    } else if (spp == 3 || spp == 4) {
        const char* layer_names[4] = {"R", "G", "B", "A"};

        for (uint16_t c = 0; c < spp; c++) {
            Imf::Slice slice = Imf::Slice::Make(
                Imf::FLOAT,
                &buffer[c],
                exr_header.dataWindow(),
                spp * sizeof(float), spp * sizeof(float) * width
            );

            exr_channels.insert(layer_names[c], Imf::Channel(Imf::FLOAT));
            exr_framebuffer.insert(layer_names[c], slice);
        }
    } else {
        throw std::runtime_error("unsupported number of samples per pixel");
    }

    Imf::OutputFile exr_out(filename.c_str(), exr_header);
    exr_out.setFrameBuffer(exr_framebuffer);
    exr_out.writePixels(height);
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

    if (!tif) {
        std::cerr << "ERR: could not open the TIFF file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t width, height;
    uint16_t bps, spp;
    uint16_t config;
    uint16_t type;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &type);

    print_tif_info(width, height, bps, spp, config, type);

    // If SamplesPerPixel is 1, PlanarConfiguration is irrelevant, and need not be included.
    if (spp == 1) { config = PLANARCONFIG_CONTIG; }

    std::vector<float> output_buffer(width * height * spp);

    try {
        switch (type) {
            case SAMPLEFORMAT_UINT:
                std::cout << "Reading sample format: unsigned int" << std::endl;

                switch( bps) {
                    case 8:
                        int_tif_to_float<uint8_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    case 16:
                        int_tif_to_float<uint16_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    case 32:
                        int_tif_to_float<uint32_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    default:
                        throw std::runtime_error("bytes per samples not supported");
                }
                break;

            case SAMPLEFORMAT_INT:
                std::cout << "Reading sample format: int" << std::endl;

                switch (bps) {
                    case 8:
                        int_tif_to_float<int8_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    case 16:
                        int_tif_to_float<int16_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    case 32:
                        int_tif_to_float<int32_t>(width, height, spp, config, tif, output_buffer);
                        break;

                    default:
                        throw std::runtime_error("bytes per samples not supported");
                }
                break;

            case SAMPLEFORMAT_IEEEFP:
                std::cout << "Reading sample format: float" << std::endl;

                switch (bps) {
                    case 16:
                        float_tif_to_float<half>(width, height, spp, config, tif, output_buffer);
                        break;

                    case 32:
                        float_tif_to_float<float>(width, height, spp, config, tif, output_buffer);
                        break;

                    default:
                        throw std::runtime_error("bytes per samples not supported");
                }
                break;

            default:
                throw std::runtime_error("format not supported");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    TIFFClose(tif);

    // TODO: double check this does also apply to > 8 bps images...
    //       in my experiments with RawThreapee, all TIFF are gamma mapped
    for (size_t i = 0; i < width * height * spp; i++) {
        output_buffer[i] = to_linear_RGB(output_buffer[i]);
    }

    try {
        std::cout << "Writing: " << argv[2] << "..." << std::endl;
        write_exr(argv[2], width, height, spp, output_buffer);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
