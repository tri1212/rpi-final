#include "kernel.h"
#include "mbox.h"
#include "uart1.h"
#include "framebf.h"
#include "image.h"
#include "game.h"

char view = '0';
int image_index = 0;
int image_offset = 0;

void main()
{
    uart_init();

    // Initialize frame buffer
    framebf_init();

    init_images();

    while (1) {
        // read each char
        char c = get_uart();

        if (c != 0)
            uart_sendc(c);

        if (c >= '0' && c <= '4') {
            view = c;

            // For case #1, display the first image automatically
            if (view == '1') {
                image_index = 0;
                image_offset = 0;
                clear_screen();
                display_image(0, 0, images[0].width, images[0].height, images[0].data, 0, 0);
            }
        }

        switch (view)
        {
        case '1':
        {
            if (c == 'A' || c == 'a')
            {
                image_index--;
                image_offset = 0;
                if (image_index < 0)
                    image_index = MAX_IMAGES - 1;
                clear_screen();
                display_image(0, 0, images[image_index].width, images[image_index].height, images[image_index].data, 0, 0);
            }
            else if (c == 'D' || c == 'd')
            {
                image_index++;
                image_offset = 0;
                if (image_index >= MAX_IMAGES)
                    image_index = 0;
                clear_screen();
                display_image(0, 0, images[image_index].width, images[image_index].height, images[image_index].data, 0, 0);
            }
            else if (c == 'W' || c == 'w')
            {
                image_offset -= 50;
                clear_screen();
                display_image(0, image_offset, images[image_index].width, images[image_index].height, images[image_index].data, 0, 0);
            }
            else if (c == 'S' || c == 's')
            {
                image_offset += 50;
                clear_screen();
                display_image(0, image_offset, images[image_index].width, images[image_index].height, images[image_index].data, 0, 0);
            }
            while (!uart_isReadByteReady());
            break;
        }

        case '2':
        {
            clear_screen();
            display_video(0, 0);
            break;
        }

        case '3':
        {
            clear_screen();
            draw_string(50, 50, "Hoang Dinh Tri - s3877818", 0x00FF0000, 2, 0.7);
            draw_string(50, 80, "Tran Truong Son - s3877818", 0x0000FF00, 2, 0.7);
            draw_string(50, 110, "Vu Thien Nhan - s3810151", 0x0000FFFF, 2, 0.7);
            draw_string(50, 140, "Phan Ngoc Quang Anh - s3810148", 0x00FFFF00, 2, 0.7);
            while (!uart_isReadByteReady());
            break;
        }

        case '4':
        {
            game();
            break;
        }

        default:
        {
            clear_screen();
            draw_string(50, 50, "Use 1, 2, 3, 4 to navigate between views", 0xFFFFFFFF, 2, 0.5);
            while (!uart_isReadByteReady());
            break;
        }
        }
    }
}
