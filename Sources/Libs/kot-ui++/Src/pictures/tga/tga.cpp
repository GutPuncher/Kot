#include "../picture.h"

namespace Ui {

    uint32_t* TGARead(TGAHeader_t* buffer) {
        uint8_t Bpp = buffer->Bpp;
        uint8_t Btpp = buffer->Bpp/8;
        uint16_t Width = buffer->Width;
        uint16_t Height = buffer->Height;
        uint32_t Pitch = Width * Btpp;
        //bool isReversed = !(buffer->ImageDescriptor & (1 << 5));

        uintptr_t ImageDataOffset = (uintptr_t) (buffer->ColorMapOrigin + buffer->ColorMapLength + 18),
            ImagePixelData = (uintptr_t) ((uint64_t)buffer + (uint64_t)ImageDataOffset);

        uint32_t* Pixels = (uint32_t*) malloc(Height * Width * sizeof(uint32_t));

        switch (buffer->ImageType)
        {
            case TGAType::COLORMAP:
                Printlog("tga type 1");
                break;
            
            case TGAType::RGB:
            {
                Printlog("tga type 2");

                for(uint16_t y = 0; y < Height; y++) {
                    //uint32_t YPosition = ((isReversed) ? Height-y-1 : y) * buffer->Height / Height;

                    for(uint16_t x = 0; x < Width; x++) {
                        //uint32_t XPosition = x * buffer->Width / Width;

                        uint64_t index = (uint64_t)ImagePixelData + x*Btpp + y*Pitch;

                        uint8_t B = *(uint8_t*) (index + 0);
                        uint8_t G = *(uint8_t*) (index + 1);
                        uint8_t R = *(uint8_t*) (index + 2);

                        uint8_t A = 0xFF;
                        if(Bpp == 32)
                            A = *(uint8_t*) (index + 3);

                        Pixels[x + y*Width] = B | (G << 8) | (R << 16) | (A << 24);
                    }
                }

                break;
            }
            case TGAType::COLORMAP_RLE:
                Printlog("tga type 9");
                break;
            
            case TGAType::RGB_RLE:
                Printlog("tga type 10");
                break;
            
            default:
                break;
        }
        return Pixels;
    }
    
}