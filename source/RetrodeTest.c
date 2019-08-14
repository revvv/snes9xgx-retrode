/*
 * RetrodeTest by REHvolution
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <wiiuse/wpad.h>

static u32 *xfb;
static GXRModeObj *rmode;

// https://www.codemii.com/2008/08/24/tutorial-3-controller-input/
void clearscreen()
{
    int w = rmode->fbWidth / 2;
    int h = rmode->xfbHeight;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            xfb[x + w * y] = COLOR_BLACK;
    printf("\x1b[2;0H");
}

void Initialise() {

    VIDEO_Init();

    // init WiiMote
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

    // init GameCube
    PAD_Init();

    rmode = VIDEO_GetPreferredMode(NULL);

    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

bool B1 = 0;
bool B2 = 0;

void handleEvent()
{
    // controller support by Mr_Reaper
    // get button states for port "0" (first player only)
    // from Wiimote
    WPAD_ScanPads();
    u32 wpaddown = WPAD_ButtonsDown(0);
    u32 wpadheld = WPAD_ButtonsHeld(0);

    // and GameCube
    PAD_ScanPads();
    u16 GCbuttonsDown = PAD_ButtonsDown(0);
    u16 GCbuttonsHeld = PAD_ButtonsHeld(0);

    // init universal variables to indicate what "button" to trigger

    // left, right, up, down movement
    bool L = 0;
    bool R = 0;
    bool U = 0;
    bool D = 0;

    // button 1, 2, Home, A, B
    // add more if you need more buttons....
    B1 = 0;
    B2 = 0;
    bool H = 0;
    bool A = 0;
    bool B = 0;

    // GameCube
    if (GCbuttonsDown & PAD_BUTTON_Y) A = 1;     // Y is treated as A on wiimote.
    if (GCbuttonsDown & PAD_TRIGGER_Z) B = 1;    // Z as B button
    if (GCbuttonsDown & PAD_BUTTON_START) H = 1; // start is treated as Home
    if (GCbuttonsDown & PAD_BUTTON_B) B1 = 1;    // B is treated as button 1
    if (GCbuttonsDown & PAD_BUTTON_A) B2 = 1;    // A is treated as button 2

    // analog stick converted to digital movement
    if (PAD_StickY(0) > 40) U = 1;
    if (PAD_StickY(0) < -40) D = 1;
    if (PAD_StickX(0) > 40) R = 1;
    if (PAD_StickX(0) < -40) L = 1;

    // digital pad on GameCube
    if (GCbuttonsHeld & PAD_BUTTON_LEFT) L = 1;
    if (GCbuttonsHeld & PAD_BUTTON_RIGHT) R = 1;
    if (GCbuttonsHeld & PAD_BUTTON_UP) U = 1;
    if (GCbuttonsHeld & PAD_BUTTON_DOWN) D = 1;

    // wiimote expansion controllers
    expansion_t e;
    int nx = 0;
    int ny = 0;

    WPAD_Expansion(WPAD_CHAN_0, &e);

    // nunchuk
    if (e.type == WPAD_EXP_NUNCHUK)
    {
        // movement with stick
        nx = e.nunchuk.js.pos.x - e.nunchuk.js.center.x;
        ny = e.nunchuk.js.pos.y - e.nunchuk.js.center.y;

        if (nx > 30) R = 1;
        if (nx < -30) L = 1;
        if (ny > 30) U = 1;
        if (ny < -30) D = 1;

        // buttons
        if (wpaddown & WPAD_NUNCHUK_BUTTON_C) B1 = 1;
        if (wpaddown & WPAD_NUNCHUK_BUTTON_Z) B2 = 1;
    }

    // classic controller
    else if (e.type == WPAD_EXP_CLASSIC)
    {
        // movement with stick
        nx = e.classic.ljs.pos.x - e.classic.ljs.center.x;
        ny = e.classic.ljs.pos.y - e.classic.ljs.center.y;

        if (nx > 10) R = 1;
        if (nx < -10) L = 1;
        if (ny > 10) U = 1;
        if (ny < -10) D = 1;

        // movement with digital pad
        if (wpadheld & WPAD_CLASSIC_BUTTON_LEFT) L = 1;
        if (wpadheld & WPAD_CLASSIC_BUTTON_RIGHT) R = 1;
        if (wpadheld & WPAD_CLASSIC_BUTTON_UP) U = 1;
        if (wpadheld & WPAD_CLASSIC_BUTTON_DOWN) D = 1;

        // whatever buttons we want to use
        if (wpaddown & WPAD_CLASSIC_BUTTON_PLUS) A = 1;   // treat + as Wiimote A button
        if (wpaddown & WPAD_CLASSIC_BUTTON_MINUS) B = 1;  // treat - as Wiimote B button
        if (wpaddown & WPAD_CLASSIC_BUTTON_HOME) H = 1;
        if (wpaddown & WPAD_CLASSIC_BUTTON_Y) B1 = 1;     // Y will be button 1
        if (wpaddown & WPAD_CLASSIC_BUTTON_B) B2 = 1;     // B will be button 2
        if (wpaddown & WPAD_CLASSIC_BUTTON_X) B1 = 1;     // X will be button 1
        if (wpaddown & WPAD_CLASSIC_BUTTON_A) B2 = 1;     // A will be button 2
    }

    // always get standard Wiimote readings
    if (wpaddown & WPAD_BUTTON_A) A = 1;
    if (wpaddown & WPAD_BUTTON_B) B = 1;
    if (wpaddown & WPAD_BUTTON_HOME) H = 1;
    if (wpaddown & WPAD_BUTTON_1) B1 = 1;
    if (wpaddown & WPAD_BUTTON_2) B2 = 1;

    // use held state for movement, not just down state which only triggers once
    if (wpadheld & WPAD_BUTTON_LEFT) D = 1;
    if (wpadheld & WPAD_BUTTON_RIGHT) U = 1;
    if (wpadheld & WPAD_BUTTON_UP) L = 1;
    if (wpadheld & WPAD_BUTTON_DOWN) R = 1;

    // then just have code respond based on what has been detected, L, R, U, D, BA, B1, B2, H.....
    // values will be "1" or "0," true or false
    // so like, just check if R = 1 and if so, move Right, etc.

    if (A || B || B1 || B2 || L || R || U || D); // -Wunused-but-set-variable

    if (H)
    {
        exit(0);
    }

}

void printUSB_DevDesc(usb_devdesc devdesc)
{
    // https://www.beyondlogic.org/usbnutshell/usb5.shtml

    // the values of my devices in comments: Hornet, Retrode Gampads 1-4 / Retrode SNES Mouse

    printf("USB Device Descriptor\n");
    printf("---------------------\n");
    printf("bLength: %d\n",            devdesc.bLength);
    printf("bDescriptorType: %d\n",    devdesc.bDescriptorType);
    printf("bcdUSB: %d\n",             devdesc.bcdUSB); // 256, 272 (Hornet,Retrode/Retrode)
    printf("bDeviceClass: %d\n",       devdesc.bDeviceClass); // 0, 0
    printf("bDeviceSubClass: %d\n",    devdesc.bDeviceSubClass); // 0, 0
    printf("bDeviceProtocol: %d\n",    devdesc.bDeviceProtocol); // 0, 0
    printf("bMaxPacketSize0: %d\n",    devdesc.bMaxPacketSize0); // important // 8, 8
    printf("idVendor: 0x%04X\n",       devdesc.idVendor);
    printf("idProduct: 0x%04X\n",      devdesc.idProduct);
    printf("bcdDevice: %d\n",          devdesc.bcdDevice); // 262, ?
    printf("iManufacturer: %d\n",      devdesc.iManufacturer); // 0, 1
    printf("iProduct: %d\n",           devdesc.iProduct); // important (for USB_GetAsciiString) // 2, 2
    printf("iSerialNumber: %d\n",      devdesc.iSerialNumber); // 0, 0
    printf("bNumConfigurations: %d\n", devdesc.bNumConfigurations); // 1, 1
    //printf("configurations: %d\n", devdesc.configurations != NULL);
}

void printUSB_ConfDesc(usb_configurationdesc confdesc)
{
    printf("USB Configuration Descriptor\n");
    printf("----------------------------\n");
    printf("bLength: %d\n",             confdesc.bLength);
    printf("bDescriptorType: %d\n",     confdesc.bDescriptorType);
    printf("wTotalLength: %d\n",        confdesc.wTotalLength); // 34, 82
    printf("bNumInterfaces: %d\n",      confdesc.bNumInterfaces); // 1, 1
    printf("bConfigurationValue: %d\n", confdesc.bConfigurationValue); // 1, 1
    printf("iConfiguration: %d\n",      confdesc.iConfiguration); // 0, 0
    printf("bmAttributes: %d\n",        confdesc.bmAttributes); // 128, 128
    printf("bMaxPower: %d\n",           confdesc.bmAttributes); // 128, 128
    //printf("interfaces: %d\n", devdesc.configurations.interfaces != NULL);
}

void printUSB_IntDesc(usb_interfacedesc intdesc)
{
    printf("USB Interface Descriptor\n");
    printf("------------------------\n");
    printf("bLength: %d\n",            intdesc.bLength);
    printf("bDescriptorType: %d\n",    intdesc.bDescriptorType);
    printf("bInterfaceNumber: %d\n",   intdesc.bInterfaceNumber); // 0, 1/2
    printf("bAlternateSetting: %d\n",  intdesc.bAlternateSetting); // 0, 0
    printf("bNumEndpoints: %d\n",      intdesc.bNumEndpoints); // 1, 1
    printf("bInterfaceClass: %d\n",    intdesc.bInterfaceClass); // 3, 3 (*)
    printf("bInterfaceSubClass: %d\n", intdesc.bInterfaceSubClass); // 0, 0/1 [I use it to distinct between SNES gamepads and SNES mouse]
    printf("bInterfaceProtocol: %d\n", intdesc.bInterfaceProtocol); // 0, 0/2 (**)
    printf("iInterface: %d\n",         intdesc.iInterface); // 0, 0
    //printf("endpoints: %d\n",         intdesc.endpoints != NULL);

    // (*) https://www.usb.org/defined-class-codes
    //     bInterfaceClass: 3 = HID (Human Interface Device)
    // (**) https://www.usb.org/sites/default/files/documents/hid1_11.pdf
    //     bInterfaceProtocol: 2 = Mouse
}

void printUSB_EndpointDesc(usb_endpointdesc endpointdesc)
{
    printf("USB Endpoint Descriptor\n");
    printf("-----------------------\n");
    printf("bLength: %d\n",              endpointdesc.bLength);
    printf("bDescriptorType: %d\n",      endpointdesc.bDescriptorType);
    printf("bEndpointAddress: 0x%02X\n", endpointdesc.bEndpointAddress); // important // 0x81, 0x86, 0x83
    printf("bmAttributes: %d\n",         endpointdesc.bmAttributes); // 0x02 means Transfer Type = Interrupt // 3, 3
    printf("wMaxPacketSize: %d\n",       endpointdesc.wMaxPacketSize); // 8, 8
    printf("bInterval: %d\n",            endpointdesc.bInterval); // 10, 10
}

void printUSB_Device(usb_devdesc devdesc)
{
    printUSB_DevDesc(devdesc);
    if (devdesc.configurations != NULL)
    {
        // only print first elements
        printUSB_ConfDesc(devdesc.configurations[0]);
        if (devdesc.configurations->interfaces != NULL)
        {
            printUSB_IntDesc(devdesc.configurations->interfaces[0]);
            if (devdesc.configurations->interfaces->endpoints != NULL)
            {
                printUSB_EndpointDesc(devdesc.configurations->interfaces->endpoints[0]);
            }
        }
    }
}

void printUSB_ProductName(s32 device_id, u8 index)
{
    if (index > 0)
    {
        uint8_t ATTRIBUTE_ALIGN(32) buf[255];
        s32 res = USB_GetAsciiString(device_id, index, 0, sizeof(buf), buf);
        if (res < 0)
        {
            printf("USB_GetAsciiString(): could not get product name\n");
        }
        else
        {
            printf("Product name: '%s'\n", buf);
        }
    }
}

bool isHornetGamepad(usb_devdesc devdesc)
{
    return (devdesc.idVendor == 0x0079 && devdesc.idProduct == 0x0011);
}

bool isRetrodeGamepad(usb_devdesc devdesc)
{
    if (devdesc.idVendor != 0x0403 || devdesc.idProduct != 0x97C1)
    {
        return false;
    }
    if (devdesc.configurations != NULL)
    {
        if (devdesc.configurations->interfaces != NULL)
        {
            if (devdesc.configurations->interfaces->endpoints != NULL)
            {
                // not sure if interface numbers are always the same, so use interface sub class
                //return devdesc.configurations->interfaces->bInterfaceNumber == 1;
                return devdesc.configurations->interfaces->bInterfaceSubClass == 0;
            }
        }
    }
    return false;
}

bool isRetrodeMouse(usb_devdesc devdesc)
{
    if (devdesc.idVendor != 0x0403 || devdesc.idProduct != 0x97C1)
    {
        return false;
    }
    if (devdesc.configurations != NULL)
    {
        if (devdesc.configurations->interfaces != NULL)
        {
            if (devdesc.configurations->interfaces->endpoints != NULL)
            {
                // not sure if interface numbers are always the same, so use interface class
                //return devdesc.configurations->interfaces->bInterfaceNumber == 2;
                return devdesc.configurations->interfaces->bInterfaceSubClass == 1;
            }
        }
    }
    return false;
}

u8 getEndpoint(usb_devdesc devdesc)
{
    if (devdesc.configurations != NULL)
    {
        if (devdesc.configurations->interfaces != NULL)
        {
            if (devdesc.configurations->interfaces->endpoints != NULL)
            {
                return devdesc.configurations->interfaces->endpoints->bEndpointAddress;
            }
        }
    }
    return -1;
}

void closeUSB(s32 *fd)
{
    s32 res = USB_CloseDevice(fd);
    if (res < 0)
    {
        printf("USB_CloseDevice: failed\n");
    }
}

// https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format/3208376#3208376
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

// poll one Retrode port (it is undefined which port is the first returned)
void testRetrodeGamepad(s32 device_id, u8 endpoint, u8 bMaxPacketSize0)
{
    //printf("device_id: %d, bEndpointAddress: 0x%02X\n", device_id, endpoint);
    uint8_t ATTRIBUTE_ALIGN(32) buf[bMaxPacketSize0];
    int res = USB_WriteIntrMsg(device_id, endpoint, sizeof(buf), buf);
    if (res < 0)
    {
        printf("Retrode: error=%d                                    \n", res);
        return;
    }

    // buf[0] contains the port returned, you have to make 4 calls to get the status,
    // even if you are only interested in one port, because it is not sure which port is returned first
    // 1 = left SNES
    // 2 = right SNES
    // 3 = left Genesis/MD
    // 4 = right Genesis/MD

    // Retrode gamepad endpoint returns 5 bytes with gamepad events

    printf("Retrode: ");

    for (int i = 0; i < res; i++)
    {
        printf("%02X ", buf[i]);
        //printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
    }

    // SNES mapping
    // A=3,10
    // B=3,01
    // X=3,20
    // Y=3,02
    // L=3,40
    // R=3,80
    // Up=2,9C
    // Down=2,64
    // Left=1,9C
    // Right=1,64
    // Start=3,08
    // Select=3,04
    // Atari Joystick Fire=3,0x03 [unused]
    // Genesis/MD layout not tested

    if (buf[3] & 0x10)
    {
        printf("A");
    }
    if (buf[3] & 0x01)
    {
        printf("B");
    }
    if (buf[3] & 0x20)
    {
        printf("X");
    }
    if (buf[3] & 0x02)
    {
        printf("Y");
    }
    if (buf[3] & 0x40)
    {
        printf("L");
    }
    if (buf[3] & 0x80)
    {
        printf("R");
    }
    if (buf[3] & 0x04)
    {
        printf("Select");
    }
    if (buf[3] & 0x08)
    {
        printf("Start");
    }
    // direction patterns overlap, therefore need exact match
    if ((buf[2] & 0x9C) == 0x9C)
    {
        printf("Up");
    }
    if ((buf[2] & 0x64) == 0x64)
    {
        printf("Down");
    }
    if ((buf[1] & 0x9C) == 0x9C)
    {
        printf("Left");
    }
    if ((buf[1] & 0x64) == 0x64)
    {
        printf("Right");
    }
    printf("                                    \n");
}

void testRetrodeMouse(s32 device_id, u8 endpoint, u8 bMaxPacketSize0)
{
    //printf("device_id: %d, bEndpointAddress: 0x%02X\n", device_id, endpoint);
    uint8_t ATTRIBUTE_ALIGN(32) buf[bMaxPacketSize0];
    int res = USB_ReadIntrMsg(device_id, endpoint, sizeof(buf), buf);
    if (res < 0)
    {
        printf("Mouse  : error=%d                                    \n", res);
        return;
    }

    // buf[0] contains the port in which a mouse was detected
    // If there is no mouse, it seems to return a random port?
    // I have no mouse to test that, sorry!
    // Retrode mouse endpoint returns 3 bytes

    printf("Mouse  : ");

    for (int i = 0; i < res; i++)
    {
        printf("%02X ", buf[i]);
        //printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
    }

    printf("                                    \n");
}

void testHornetGamepad(s32 device_id, u8 endpoint, u8 bMaxPacketSize0)
{
    //printf("device_id: %d, bEndpointAddress: 0x%02X\n", device_id, endpoint);
    uint8_t ATTRIBUTE_ALIGN(32) buf[bMaxPacketSize0];
    int res = USB_ReadIntrMsg(device_id, endpoint, sizeof(buf), buf);
    if (res < 0)
    {
        printf("error=%d                                    \n", res);
        return;
    }

    printf("Hornet : ");

    for (int i = 0; i < res; i++)
    {
        printf("%02X ", buf[i]);
        //printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
    }

    //    from my Nintendont config file: 0079_0011.ini
    //    A=5,2F
    //    B=5,1F
    //    X=6,01
    //    Y=5,8F
    //    S=6,20
    //    Up=4,00
    //    Right=3,FF
    //    Left=3,00
    //    Down=4,FF
    //    L=6,04
    //    R=6,08
    //    Z=6,02
    //    Power=6,10
    //  ;Unused=5,0x4F
    //  ;Unused=6,0x02

    // NOTE: Here's Nintendont's generic approach:
    // Create bitmask for value: atox()
    // https://github.com/FIX94/Nintendont/blob/10f52316a67d955e2c9608ae6cefc277ccfb8abe/controllerconfigs/controller_ini_to_array/main.c
    // Compare button with bitmask: Left.Mask, Right.Mask, ...
    // https://github.com/FIX94/Nintendont/blob/10f52316a67d955e2c9608ae6cefc277ccfb8abe/loader/source/ppc/PADReadGC.c

    if ((buf[5] & 0x1F) == 0x1F)
    {
        printf("1=B");
    }
    if ((buf[5] & 0x2F) == 0x2F)
    {
        printf("2=A");
    }
    if ((buf[5] & 0x4F) == 0x4F)
    {
        printf("3");
    }
    if ((buf[5] & 0x8F) == 0x8F)
    {
        printf("4=Y");
    }
    if ((buf[6] & 0x01) == 0x01)
    {
        printf("5=X");
    }
    if ((buf[6] & 0x02) == 0x02)
    {
        printf("6");
    }
    if ((buf[6] & 0x04) == 0x04)
    {
        printf("7=L");
    }
    if ((buf[6] & 0x08) == 0x08)
    {
        printf("8=R");
    }
    if ((buf[6] & 0x10) == 0x10)
    {
        printf("9=Select");
    }
    if ((buf[6] & 0x20) == 0x20)
    {
        printf("10=Start");
    }
    if (buf[4] == 0x00)
    {
        printf("Up");
    }
    if (buf[4] == 0xFF)
    {
        printf("Down");
    }
    if (buf[3] == 0x00)
    {
        printf("Left");
    }
    if (buf[3] == 0xFF)
    {
        printf("Right");
    }
    printf("                                    \n");
}

void printHeader()
{
    clearscreen();
    printf("Press 1 to open Retrode devices, press 2 to close.\n");
    printf("Retrode 1: waiting for events on device 1...\n");
    printf("Retrode 2: waiting for events on device 2...\n");
    printf("Retrode 3: waiting for events on device 3...\n");
    printf("Retrode 4: waiting for events on device 4...\n");
    printf("Mouse    : waiting for mouse events...\n");
    printf("\n\n");
}

s32 deviceIdRetrode = 0;
u8 endpointRetrode = 0;
u8 bMaxPacketSizeRetrode = 0;

s32 deviceIdRetrodeMouse = 0;
u8 endpointRetrodeMouse = 0;
u8 bMaxPacketSizeRetrodeMouse = 0;

s32 deviceIdHornet = 0;
u8 endpointHornet = 0;
u8 bMaxPacketSizeHornet = 0;

// Close USB gamepads
void closeUSB_Gamepads()
{
    if (deviceIdRetrode != 0)
    {
        closeUSB(&deviceIdRetrode);
        deviceIdRetrode = 0;
        endpointRetrode = 0;
        bMaxPacketSizeRetrode = 0;
    }
    if (deviceIdRetrodeMouse != 0)
    {
        closeUSB(&deviceIdRetrodeMouse);
        deviceIdRetrodeMouse = 0;
        endpointRetrodeMouse = 0;
        bMaxPacketSizeRetrodeMouse = 0;
    }
    if (deviceIdHornet != 0)
    {
        closeUSB(&deviceIdHornet);
        deviceIdHornet = 0;
        endpointHornet = 0;
        bMaxPacketSizeHornet = 0;
    }
}

int main1() {
    Initialise();
    //IOS_ReloadIOS(58);
    //usleep(100 * 1000);
    //USB_Initialize();
    printHeader();

    while (true)
    {
        handleEvent();
        if (B1)
        {
            printHeader();
            closeUSB_Gamepads();

            //printf("USB_GetDeviceList()\n");
            usb_device_entry dev_entry[8];
            u8 dev_count;
            if (USB_GetDeviceList(dev_entry, 8, USB_CLASS_HID, &dev_count) < 0)
            {
                printf("USB_GetDeviceList(): error\n");
                continue;
            }

            printf("Found %d devices\n", dev_count);

            // Retrode has two entries in USB_GetDeviceList(), one for gamepads and one for SNES mouse
            for (int i = 0; i < dev_count; ++i)
            {
                printf("----------------------------------------------\n");
                s32 fd;
                printf("Device #%d: vid=0x%04X pid=0x%04X id=0x%08X\n", i, dev_entry[i].vid, dev_entry[i].pid, dev_entry[i].device_id);
                if (USB_OpenDevice(dev_entry[i].device_id, dev_entry[i].vid, dev_entry[i].pid, &fd) < 0)
                {
                    printf("USB_OpenDevice(): failed\n");
                    continue;
                }

                usb_devdesc devdesc;
                if (USB_GetDescriptors(fd, &devdesc) < 0)
                {
                    printf("USB_GetDescriptors(): failed [Maybe not enough power? Is another gamepad connected?]\n");
                    closeUSB(&fd);
                    continue;
                }
                //printUSB_Device(devdesc);
                printUSB_ProductName(fd, devdesc.iProduct);
                printf("bEndpointAddress: 0x%02X\n", getEndpoint(devdesc));
                printf("bMaxPacketSize0: %d\n", devdesc.bMaxPacketSize0);
                if (isRetrodeGamepad(devdesc))
                {
                    printf("Retrode gamepads\n");
                    deviceIdRetrode = fd;
                    endpointRetrode = getEndpoint(devdesc);
                    bMaxPacketSizeRetrode = devdesc.bMaxPacketSize0;
                }
                else if (isRetrodeMouse(devdesc))
                {
                    printf("Retrode SNES mouse\n");
                    deviceIdRetrodeMouse = fd;
                    endpointRetrodeMouse = getEndpoint(devdesc);
                    bMaxPacketSizeRetrodeMouse = devdesc.bMaxPacketSize0;
                }
                else if (isHornetGamepad(devdesc))
                {
                    printf("Hornet Gamepad\n");
                    deviceIdHornet = fd;
                    endpointHornet = getEndpoint(devdesc);
                    bMaxPacketSizeHornet = devdesc.bMaxPacketSize0;
                }
                else
                {
                    closeUSB(&fd);
                }
            }
        }
        if (B2)
        {
            printHeader();
            closeUSB_Gamepads();
        }

        if (deviceIdRetrode != 0)
        {
            printf("\x1b[3;0H");
            // poll all four ports
            testRetrodeGamepad(deviceIdRetrode, endpointRetrode, bMaxPacketSizeRetrode);
            testRetrodeGamepad(deviceIdRetrode, endpointRetrode, bMaxPacketSizeRetrode);
            testRetrodeGamepad(deviceIdRetrode, endpointRetrode, bMaxPacketSizeRetrode);
            testRetrodeGamepad(deviceIdRetrode, endpointRetrode, bMaxPacketSizeRetrode);
        }
        if (deviceIdRetrodeMouse != 0)
        {
            printf("\x1b[7;0H");
            testRetrodeMouse(deviceIdRetrodeMouse, endpointRetrodeMouse, bMaxPacketSizeRetrodeMouse);
        }
        if (deviceIdHornet != 0)
        {
            printf("\x1b[8;0H");
            testHornetGamepad(deviceIdHornet, endpointHornet, bMaxPacketSizeHornet);
        }
    }

    return 0;
}

