#include "AXS15231B.h"              // Custom display driver header
#include "SPI.h"                    // SPI communication library
#include "Arduino.h"                // Arduino core library
#include "driver/spi_master.h"      // ESP-IDF SPI driver

/**
 * This requires quite a bit of cleaning/improvement. TODO this gently and gradually in the future.
 */

// Allocating a buffer in PSRAM (external memory) for matrix rotation (640 * 180 * 2 bytes for RGB565 color)
uint16_t* qBuffer = (uint16_t*) heap_caps_malloc(230400, MALLOC_CAP_SPIRAM); //psram buffer for matrix rotation (640 * 180 * 2)

// Flag to track the state of SPI DMA (Direct Memory Access) writing to the display
static volatile bool lcd_spi_dma_write = false;
uint32_t transfer_num = 0;          // Tracks ongoing SPI transfers
size_t lcd_PushColors_len = 0;      // Tracks the length of the data being pushed to the display

// Initialization sequence for the AXS15231B display (commands and data to be sent via SPI)
const static lcd_cmd_t axs15231b_qspi_init[] = {
    {0x28, {0x00}, 0x40},   // Command 0x28 with flags (e.g., 0x40 for delays)
    {0x10, {0x00}, 0x20},
    {0x11, {0x00}, 0x80},
    {0x29, {0x00}, 0x00}, 
};

// Getter for the SPI DMA write flag
bool get_lcd_spi_dma_write(void)
{
    return lcd_spi_dma_write;
}

// SPI device handle, used to communicate with the display
static spi_device_handle_t spi;

// Function to send a command to the display over SPI
static void WriteComm(uint8_t data)
{
    TFT_CS_L;       // Lower the chip select line to start communication
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));   // Start SPI transaction
    SPI.write(0x00);        // Send a placeholder byte
    SPI.write(data);        // Send the actual command byte
    SPI.write(0x00);        // Another placeholder byte
    SPI.endTransaction();   // End SPI transaction
    TFT_CS_H;       // Raise chip select to end communication
}

// Function to send data to the display over SPI
static void WriteData(uint8_t data)
{
    TFT_CS_L;
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));   // Start SPI transaction
    SPI.write(data);            // Send data byte
    SPI.endTransaction();       // End SPI transaction
    TFT_CS_H;
}

// Function to send commands and data to the display with additional options (for SPI/DMA)
static void lcd_send_cmd(uint32_t cmd, uint8_t *dat, uint32_t len)
{
    TFT_CS_L;               // Lower the chip select line
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));           // Clear the SPI transaction structure
    t.flags = (SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR);     // Define SPI transfer flags
    #ifdef LCD_SPI_DMA      // Check if DMA mode is enabled
        if(cmd == 0xff && len == 0x1f)  // Special case for command 0xff
        {
            t.cmd = 0x02;               // Command to be sent
            t.addr = 0xffff;            // Address to be sent
            len = 0;                    // No data for this command
        }
        else if(cmd == 0x00)
        {
            t.cmd = 0X00;               // Another special case
            t.addr = 0X0000;
            len = 4;                    // Sending 4 bytes of data
        }
        else 
        {
            t.cmd = 0x02;               // General case for sending a command
            t.addr = cmd << 8;          // Shift command bits for address mode
        }
    #else                   // Non-DMA case
        t.cmd = 0x02;                   // Command to send data over SPI
        t.addr = cmd << 8;
    #endif
    if (len != 0) {         // If there's data to send
        t.tx_buffer = dat;              // Set the data buffer
        t.length = 8 * len;             // Set the data length in bits
    } else {                // No data to send
        t.tx_buffer = NULL;
        t.length = 0;
    }
    spi_device_polling_transmit(spi, &t);       // Transmit the data over SPI
    TFT_CS_H;       // Raise chip select after the transfer
    if(0)
    {
        WriteComm(cmd);
        if (len != 0) {
            for (int i = 0; i < len; i++)
                WriteData(dat[i]);
        }
    }
}

// static void IRAM_ATTR spi_dma_cd(spi_transaction_t *trans)
// {
//     if(transfer_num > 0)
//     {
//         transfer_num--;
//     }
        
//     if(lcd_PushColors_len <= 0 && transfer_num <= 0)
//     {
//         if(lcd_spi_dma_write) {
//             lcd_spi_dma_write = false;
//             lv_disp_t * disp = _lv_refr_get_disp_refreshing();
//             if(disp != NULL)
//                 lv_disp_flush_ready(disp->driver);

//             TFT_CS_H;
//         }
//     }
// }

// Initialization of the AXS15231B display
void axs15231_init(void)
{

    pinMode(TFT_QSPI_CS, OUTPUT);       // Set the Chip Select pin as output
    pinMode(TFT_QSPI_RST, OUTPUT);      // Set the Reset pin as output

    // Reset the display
    TFT_RES_H;      // Set reset high
    delay(130);     // Wait for 130 ms
    TFT_RES_L;      // Set reset low
    delay(130);     // Wait for another 130 ms
    TFT_RES_H;      // Set reset high again
    delay(300);     // Wait for the display to stabilize

    esp_err_t ret;

    // Initialize SPI bus configuration
    spi_bus_config_t buscfg = {
        .data0_io_num = TFT_QSPI_D0,        // Data line 0 pin
        .data1_io_num = TFT_QSPI_D1,        // Data line 1 pin
        .sclk_io_num = TFT_QSPI_SCK,        // Clock pin
        .data2_io_num = TFT_QSPI_D2,        // Data line 2 pin
        .data3_io_num = TFT_QSPI_D3,        // Data line 3 pin
        .max_transfer_sz = (SEND_BUF_SIZE * 16) + 8,        // Maximum transfer size in bytes
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS /* |            // SPI master mode and GPIO pins
                 SPICOMMON_BUSFLAG_QUAD */
        ,
    };

    // SPI device configuration
    spi_device_interface_config_t devcfg = {
        .command_bits = 8,      // Command is 8 bits long
        .address_bits = 24,     // Address is 24 bits long
        .mode = TFT_SPI_MODE,   // SPI mode
        .clock_speed_hz = SPI_FREQUENCY,    // SPI clock frequency
        .spics_io_num = -1,     // CS pin (chip select)
        // .spics_io_num = TFT_QSPI_CS,
        .flags = SPI_DEVICE_HALFDUPLEX,     // Half-duplex SPI communication
        .queue_size = 17,       // Queue size for transactions
//        .post_cb = spi_dma_cd,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(TFT_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);   // Check for errors in SPI initialization

    // Add the SPI device to the bus
    ret = spi_bus_add_device(TFT_SPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);   // Check for errors in adding the SPI device


    // Run the initialization sequence for the display: Initialize the screen multiple times to prevent initialization failure
    
    int i = 1;  // Retry loop for display initialization
    while (i--) {

        const lcd_cmd_t *lcd_init = axs15231b_qspi_init;
        for (int i = 0; i < sizeof(axs15231b_qspi_init) / sizeof(lcd_cmd_t); i++)

        {
            lcd_send_cmd(lcd_init[i].cmd,
                         (uint8_t *)lcd_init[i].data,
                         lcd_init[i].len & 0x3f);

            if (lcd_init[i].len & 0x80)
                delay(200);
            if (lcd_init[i].len & 0x40)
                delay(20);
        }
    }
}

// Function to set the screen rotation
void lcd_setRotation(uint8_t r)
{
    uint8_t gbr = TFT_MAD_RGB;      // MADCTL parameter for RGB format

    switch (r) {
    case 0: // Portrait mode
        // WriteData(gbr);      // Not implemented here
        break;
    case 1: // Landscape mode (Portrait + 90 degrees)
        gbr = TFT_MAD_MX | TFT_MAD_MV | gbr;
        break;
    case 2: // Inverter portrait
        gbr = TFT_MAD_MX | TFT_MAD_MY | gbr;
        break;
    case 3: // Inverted landscape
        gbr = TFT_MAD_MV | TFT_MAD_MY | gbr;
        break;
    }
    // Send the MADCTL (Memory Access Control) command to set the orientation
    lcd_send_cmd(TFT_MADCTL, &gbr, 1);
}

// Function to set the drawing region (window) on the screen
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_cmd_t t[3] = {
        {0x2a, {(uint8_t)(x1 >> 8), (uint8_t)x1, (uint8_t)(x2 >> 8), (uint8_t)(x2)}, 0x04},
        {0x2b, {(uint8_t)(y1 >> 8), (uint8_t)(y1), (uint8_t)(y2 >> 8), (uint8_t)(y2)}, 0x04},
    };

    // Send the address setting commands
    for (uint32_t i = 0; i < 2; i++) {
        lcd_send_cmd(t[i].cmd, t[i].data, t[i].len);
    }
}

// Function to fill a rectangular area with a single color
void lcd_fill(uint16_t xsta,
              uint16_t ysta,
              uint16_t xend,
              uint16_t yend,
              uint16_t color)
{

    uint16_t w = xend - xsta;   // Calculate width of the rectangle
    uint16_t h = yend - ysta;   // Calculate height of the rectangle
    uint16_t *color_p = (uint16_t *)heap_caps_malloc(w * h * 2, MALLOC_CAP_INTERNAL);   // Allocate memory for the color data
    int i = 0;

    // Fill the buffer with the color value
    for(i = 0; i < w * h ; i+=1)
    {
        color_p[i] = color;
    }

    // Send the color data to the display
    lcd_PushColors(xsta, ysta, w, h, color_p);
    free(color_p);      // Free the allocated memory
}

void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // Create a temporary buffer large enough to hold the rectangle's color data
    static uint16_t colorBuffer[SEND_BUF_SIZE];

    // Fill the color buffer with the specified color
    for (size_t i = 0; i < SEND_BUF_SIZE; i++) {
        colorBuffer[i] = color;
    }

    size_t totalPixels = w * h;
    size_t pixelsRemaining = totalPixels;
    uint16_t *bufferPointer = colorBuffer;

    // Loop to send the entire rectangle in chunks if necessary
    while (pixelsRemaining > 0) {
        // Determine how many pixels to send in this iteration
        size_t chunkSize = (pixelsRemaining > SEND_BUF_SIZE) ? SEND_BUF_SIZE : pixelsRemaining;

        // Call lcd_PushColors_rotated_90 with the current buffer
        lcd_PushColors_rotated_90(x, y, w, chunkSize / w, bufferPointer);

        // Update the remaining pixels count
        pixelsRemaining -= chunkSize;
    }
}

// Function to draw a single pixel on the screen
void lcd_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_address_set(x, y, x + 1, y + 1);        // Set the address for the pixel
    lcd_PushColors(&color, 1);                  // Push the pixel color to the display
}

// Wrapper function to queue an SPI transfer
void spi_device_queue_trans_fun(spi_device_handle_t handle, spi_transaction_t *trans_desc, TickType_t ticks_to_wait)
{
    // Queue the SPI transaction and check for errors
    ESP_ERROR_CHECK(spi_device_queue_trans(spi, (spi_transaction_t *)trans_desc, portMAX_DELAY));
}

// Function to send color data to the display using SPI and DMA (when enabled)
#ifdef LCD_SPI_DMA 
spi_transaction_ext_t t = {0};              // Extended SPI transaction structure
void lcd_PushColors(uint16_t x,
                        uint16_t y,
                        uint16_t width,
                        uint16_t high,
                        uint16_t *data)
    {
        static bool first_send = 1;             // Flag to check if it's the first data send
        static uint16_t *p = (uint16_t *)data;  // Pointer to the color data
        static uint32_t transfer_num_old = 0;   // Old transfer count

        // If valid data is provided
        if(data != NULL && (width != 0) && (high != 0))
        {
            lcd_PushColors_len = width * high;
            p = (uint16_t *)data;
            first_send = 1;

            transfer_num = 0;

            // Set the drawing window
            lcd_address_set(x, y, x + width - 1, y + high - 1);
            TFT_CS_L;       // Lower the chip select to start SPI communication
        }

        // Handle completed transfers
        for (int x = 0; x < (transfer_num_old - (transfer_num_old-(transfer_num_old-transfer_num))); x++) {
            spi_transaction_t *rtrans;
            esp_err_t ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
            if (ret != ESP_OK) {
            // ESP_LOGW(TAG, "1. transfer_num = %d", transfer_num_old);
            }
            assert(ret == ESP_OK);      // Ensure the transfer was successful
        }
        transfer_num_old -= (transfer_num_old - (transfer_num_old-(transfer_num_old-transfer_num)));

        // Start sending data chunks
        do {
            if(transfer_num >= 3 || ESP.getFreeHeap() <= 70000)
            {
                break;
            }
            size_t chunk_size = lcd_PushColors_len;

            memset(&t, 0, sizeof(t));
            if (first_send) {
                t.base.flags =
                    SPI_TRANS_MODE_QIO ;// | SPI_TRANS_MODE_DIOQIO_ADDR         // Quad I/O transfer mode
                t.base.cmd = 0x32 ;// 0x12      // Command to write to memory
                t.base.addr = 0x002C00;         // Starting address
                first_send = 0;
            } else {
                t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD |
                            SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
                t.command_bits = 0;
                t.address_bits = 0;
                t.dummy_bits = 0;
            }
            if (chunk_size > SEND_BUF_SIZE) {
                chunk_size = SEND_BUF_SIZE;         // Limit the chunk size
            }
            t.base.tx_buffer = p;                   // Set the data buffer
            t.base.length = chunk_size * 16;        // Set the data length

            lcd_spi_dma_write = true;               // Mark that DMA is in use

            transfer_num++;
            transfer_num_old++;
            lcd_PushColors_len -= chunk_size;

            // Queue the SPI transfer
            esp_err_t ret;

            ESP_ERROR_CHECK(spi_device_queue_trans(spi, (spi_transaction_t *)&t, portMAX_DELAY));
            assert(ret == ESP_OK);

            p += chunk_size;        // Move to the next chunk of data
        } while (lcd_PushColors_len > 0);       // Continue until all data is sent
    }
 
#else       // Non-DMA version of the function

    void lcd_PushColors(uint16_t x,
                    uint16_t y,
                    uint16_t width,
                    uint16_t high,
                    uint16_t *data)
    {
        size_t len = width * high;          // Total length of the data
        uint16_t *p = (uint16_t *)data;
        size_t chunk_size;
        bool first_send = true;             // Flag to check if it's the first data send
        int aaa = 0;                        // Dummy variable for delay

        // Set the drawing window
        lcd_address_set(x, y, x + width - 1, y + high - 1);

        // SPI transaction structure
        spi_transaction_ext_t t = {0};
        t.base.flags = SPI_TRANS_MODE_QIO;
        t.base.cmd = 0x32;  // Command to write to memory

        // Set the first address
        t.base.addr = 0x002C00;

        // Start sending data in chunks
        do {
            TFT_CS_L;  // Lower chip select to start SPI communication

            // Adjust chunk size if necessary
            chunk_size = (len > SEND_BUF_SIZE) ? SEND_BUF_SIZE : len;

            // Set the data buffer and length
            t.base.tx_buffer = p;
            t.base.length = chunk_size * 16;  // Data length in bits (16 bits per pixel)

            // Micro time delays for ensuring proper timing
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            
            // Transmit the data
            spi_device_polling_transmit(spi, (spi_transaction_t *)&t);

            // After the first transmission, adjust the address for subsequent transfers
            if (first_send) {
                first_send = false;
                t.base.addr = 0x003C00;  // Update to the new address for subsequent transfers
            }

            // Micro time delays before toggling chip select
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;

            TFT_CS_H;  // Raise chip select

            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;

            TFT_CS_L;  // Lower chip select for the next transaction

            // Micro time delay
            aaa = aaa >> 1;
            aaa = aaa >> 1;
            aaa = aaa >> 1;

            // Update remaining length and pointer
            len -= chunk_size;
            p += chunk_size;

        } while (len > 0);  // Continue until all data is sent

        TFT_CS_H;  // Raise chip select to end SPI communication
    }
#endif

// Function to push color data to the display
void lcd_PushColors(uint16_t *data, uint32_t len)
{
    bool first_send = 1;                // Flag to track the first send
    uint16_t *p = (uint16_t *)data;     // Pointer to the color data
    TFT_CS_L;                           // Lower chip select to start SPI communication
    do {
        size_t chunk_size = len;        // Set chunk size
        spi_transaction_ext_t t = {0};  // SPI transaction structure
        memset(&t, 0, sizeof(t));       // Clear the structure
        if (first_send) {               // If it's the first transfer
            t.base.flags =
                SPI_TRANS_MODE_QIO /* | SPI_TRANS_MODE_DIOQIO_ADDR */;      // Quad I/O transfer mode
            t.base.cmd = 0x32 /* 0x12 */;       // Command to write to memory
            t.base.addr = 0x002C00;             // Set address
            first_send = 0;
        } else {
            t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD |
                           SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
            t.command_bits = 0;
            t.address_bits = 0;
            t.dummy_bits = 0;
        }

        // Adjust chunk size if necessary
        if (chunk_size > SEND_BUF_SIZE) {
            chunk_size = SEND_BUF_SIZE;
        }
        t.base.tx_buffer = p;               // Set the data buffer
        t.base.length = chunk_size * 16;    // Set the data length

        // Transmit the data
        spi_device_polling_transmit(spi, (spi_transaction_t *)&t);
        len -= chunk_size;      // Decrease the remaining length
        p += chunk_size;        // Move the pointer to the next chunk
    } while (len > 0);      // Continue until all data is sent
    TFT_CS_H;   // Raise chip select to end SPI communication
}

// Function to push rotated color data to the display
void lcd_PushColors_rotated_90(
                    uint16_t x,
                    uint16_t y,
                    uint16_t width,
                    uint16_t high,
                    uint16_t *data)
{
    uint16_t  _x = 180 - (y + high);        // Adjust coordinates for 90-degree rotation
    uint16_t  _y = x;
    uint16_t  _h = width;
    uint16_t  _w = high;

    // Set the drawing window with the rotated coordinates
    lcd_address_set(_x, _y, _x + _w - 1, _y + _h - 1);

    bool first_send = 1;
    size_t len = width * high;
    uint16_t *p = (uint16_t *)data;
    uint16_t *q = (uint16_t *)qBuffer;      // Use buffer for rotation
    uint32_t index = 0;                     // Index for the buffer
   
    // Rotate the data and store it in the buffer
    for (uint16_t j = 0; j < width; j++)
    {
        for (uint16_t i = 0; i < high; i++)
        {
            qBuffer[index++] = ((uint16_t)p[width * (high - i - 1) + j]);             
        }
    }


    TFT_CS_L;       // Lower chip select to start SPI communication
    do
    {
        size_t chunk_size = len;            // Set chunk size
        spi_transaction_ext_t t = {0};      // SPI transaction structure
        memset(&t, 0, sizeof(t));           // Clear the structure
        if (first_send)
        {
            t.base.flags =
                SPI_TRANS_MODE_QIO; // | SPI_TRANS_MODE_DIOQIO_ADDR ;      // Quad I/O transfer mode
            t.base.cmd = 0x32; // 0x12 ;       // Command to write to memory
            t.base.addr = 0x002C00;             // Set address
            first_send = 0;
        }
        else
        {
            t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD |
                           SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
            t.command_bits = 0;
            t.address_bits = 0;
            t.dummy_bits = 0;
        }

        // Adjust chunk size if necessary
        if (chunk_size > SEND_BUF_SIZE)
        {
            chunk_size = SEND_BUF_SIZE;
        }
        t.base.tx_buffer = q;               // Set the data buffer (rotated data)
        t.base.length = chunk_size * 16;    // Set the data length

        // Transmit the data
        spi_device_polling_transmit(spi, (spi_transaction_t *)&t);
        len -= chunk_size;      // Decrease the remaining length
        q += chunk_size;        // Move the pointer to the next chunk
    } while (len > 0);      // Continue until all data is sent
    TFT_CS_H;       // Raise chip select to end SPI communication
}

// Put the display to sleep
void lcd_sleep()
{
    lcd_send_cmd(0x10, NULL, 0);    // Send sleep command
}

// Set the display brightness
void hw_set_brightness(uint8_t val)
{
    lcd_send_cmd(0x51, &val, 1);    // Send brightness level
}

// Fill the screen with a specific color
void hw_colour_fill(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t rgb[] = {r,g,b};    // RGB color array
    lcd_send_cmd(0x2f, rgb, 3); // Send fill color command
}

// Clear the screen and fill with black
void hw_clear_screen_black()
{
    lcd_send_cmd(0x22, NULL, 0);    // Send command to fill the screen with black
}

