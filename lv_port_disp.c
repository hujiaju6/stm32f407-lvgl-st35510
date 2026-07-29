#include "lv_port_disp.h" /* 如果你改了头文件名，请对应修改 */
#include "lvgl.h"
#include "bsp_nt35510_lcd.h"

#define MY_DISP_HOR_RES 480
#define MY_DISP_VER_RES 800

/* LVGL 8.x 刷屏回调函数 */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t len = w * h;

    /* 1. 设置 LCD 写入窗口 */
    NT35510_OpenWindow(area->x1, area->y1, w, h);
    
    /* 2. 发送写显存命令 */
    NT35510_Write_Cmd(CMD_SetPixel);

    /* 3. FSMC 极速直写 */
    for(uint32_t i = 0; i < len; i++) {
        *(__IO uint16_t *)(FSMC_Addr_NT35510_DATA) = color_p->full;
        color_p++;
    }

    /* 4. 通知 LVGL 刷新已完成 */
    lv_disp_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    /* 初始化硬件 */
    NT35510_Init();

    /* 分配缓冲区：这里分配 60 行的高度 (480*60*2 = 57.6KB)，F407 内存完全够用 */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 60]; 
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 60);

    /* 初始化并注册 LVGL 8.x 显示驱动 */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc_1;

    lv_disp_drv_register(&disp_drv);
}