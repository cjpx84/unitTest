#include <common.h>
#include <asm/arch/mux.h>
#include "am335x-fb.h"

#define SOC_GPIO_3_REGS                      (0x481AE000)
#define SOC_GPIO_1_REGS                      (0x4804C000)
#define SOC_GPIO_0_REGS                      (0x44E07000)

#define SOC_PWMSS0_REGS                    (0x48000000+0x300000)
#define SOC_PWMSS1_REGS                    (0x48000000+0x302000)
#define SOC_PWMSS2_REGS                    (0x48000000+0x304000)

#define SOC_ECAP_REGS                          (0x00000100)
#define SOC_EQEP_REGS                          (0x00000180)
#define SOC_EPWM_REGS                         (0x00000200)
#define SOC_ECAP_0_REGS                     (SOC_PWMSS0_REGS + SOC_ECAP_REGS)
#define SOC_ECAP_1_REGS                     (SOC_PWMSS1_REGS + SOC_ECAP_REGS)
#define SOC_ECAP_2_REGS                     (SOC_PWMSS2_REGS + SOC_ECAP_REGS)

#define BIT(x)				(1 << x)
#define CL_BIT(x)			(0 << x)

/* LCD Status Register */
#define LCD_END_OF_FRAME1		BIT(9)
#define LCD_END_OF_FRAME0		BIT(8)
#define LCD_PL_LOAD_DONE		BIT(6)
#define LCD_FIFO_UNDERFLOW		BIT(5)
#define LCD_SYNC_LOST			BIT(2)

/* LCD DMA Control Register */
#define LCD_DMA_BURST_SIZE(x)		((x) << 4)
#define LCD_DMA_BURST_1			0x0
#define LCD_DMA_BURST_2			0x1
#define LCD_DMA_BURST_4			0x2
#define LCD_DMA_BURST_8			0x3
#define LCD_DMA_BURST_16		0x4
#define LCD_V1_END_OF_FRAME_INT_ENA	BIT(2)
#define LCD_V2_END_OF_FRAME0_INT_ENA	BIT(8)
#define LCD_V2_END_OF_FRAME1_INT_ENA	BIT(9)
#define LCD_DUAL_FRAME_BUFFER_ENABLE	BIT(0)

/* LCD Control Register */
#define LCD_CLK_DIVISOR(x)		((x) << 8)
#define LCD_RASTER_MODE			0x01

/* LCD Raster Control Register */
#define LCD_PALETTE_LOAD_MODE(x)	((x) << 20)
#define PALETTE_AND_DATA		0x00
#define PALETTE_ONLY			0x01
#define DATA_ONLY			0x02

#define LCD_MONO_8BIT_MODE		BIT(9)
#define LCD_RASTER_ORDER		BIT(8)
#define LCD_TFT_MODE			BIT(7)
#define LCD_V1_UNDERFLOW_INT_ENA	BIT(6)
#define LCD_V2_UNDERFLOW_INT_ENA	BIT(5)
#define LCD_V1_PL_INT_ENA		BIT(4)
#define LCD_V2_PL_INT_ENA		BIT(6)
#define LCD_MONOCHROME_MODE		BIT(1)
#define LCD_RASTER_ENABLE		BIT(0)
#define LCD_TFT_ALT_ENABLE		BIT(23)
#define LCD_STN_565_ENABLE		BIT(24)
#define LCD_V2_DMA_CLK_EN		BIT(2)
#define LCD_V2_LIDD_CLK_EN		BIT(1)
#define LCD_V2_CORE_CLK_EN		BIT(0)
#define LCD_V2_LPP_B10			26
#define LCD_V2_TFT_24BPP_MODE		BIT(25)
#define LCD_V2_TFT_24BPP_UNPACK		BIT(26)

/* LCD Raster Timing 2 Register */
#define LCD_AC_BIAS_TRANSITIONS_PER_INT(x)	((x) << 16)
#define LCD_AC_BIAS_FREQUENCY(x)		((x) << 8)
#define LCD_SYNC_CTRL				BIT(25)
#define LCD_SYNC_EDGE				BIT(24)
#define LCD_INVERT_PIXEL_CLOCK			BIT(22)
#define LCD_INVERT_LINE_CLOCK			BIT(21)
#define LCD_INVERT_FRAME_CLOCK			BIT(20)

/* LCD Block */
#define  LCD_PID_REG				0x0
#define  LCD_CTRL_REG				0x4
#define  LCD_STAT_REG				0x8
#define  LCD_RASTER_CTRL_REG			0x28
#define  LCD_RASTER_TIMING_0_REG		0x2C
#define  LCD_RASTER_TIMING_1_REG		0x30
#define  LCD_RASTER_TIMING_2_REG		0x34
#define  LCD_DMA_CTRL_REG			0x40
#define  LCD_DMA_FRM_BUF_BASE_ADDR_0_REG	0x44
#define  LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG	0x48
#define  LCD_DMA_FRM_BUF_BASE_ADDR_1_REG	0x4C
#define  LCD_DMA_FRM_BUF_CEILING_ADDR_1_REG	0x50

/* Interrupt Registers available only in Version 2 */
#define  LCD_RAW_STAT_REG			0x58
#define  LCD_MASKED_STAT_REG			0x5c
#define  LCD_INT_ENABLE_SET_REG			0x60
#define  LCD_INT_ENABLE_CLR_REG			0x64
#define  LCD_END_OF_INT_IND_REG			0x68

/* Clock registers available only on Version 2 */
#define  LCD_CLK_ENABLE_REG			0x6c
#define  LCD_CLK_RESET_REG			0x70
#define  LCD_CLK_MAIN_RESET			BIT(3)

#define LCD_NUM_BUFFERS	2

#define WSI_TIMEOUT	50
#define PALETTE_SIZE	256
#define LEFT_MARGIN	64
#define RIGHT_MARGIN	64
#define UPPER_MARGIN	32
#define LOWER_MARGIN	32

#define SOC_LCDC_0_REGS                     (0x4830E000)
#define lcdc_read(reg_offset)           readl(SOC_LCDC_0_REGS+(reg_offset))      
#define lcdc_write(val,reg_offset)    writel((val),SOC_LCDC_0_REGS+(reg_offset))

#define periodCnt        ((100000000)/(pwmFreq))

enum panel_type {
	QVGA = 0,
	WVGA
};

enum panel_shade {
	MONOCHROME = 0,
	COLOR_ACTIVE,
	COLOR_PASSIVE
};

enum raster_load_mode {
	LOAD_DATA = 1,
	LOAD_PALETTE
};

static volatile unsigned short*  framebuf = LCD_BASE_FRAME_ADDR;
static volatile unsigned int*  lcdc_base =	(volatile unsigned int*)SOC_LCDC_0_REGS;
static int pwmFreq = 1000;  //1k

extern void udelay(unsigned long usec);

/* Module pin mux for LCDC */
static struct module_pin_mux  lcdc_pin_mux[] = {
	{AM33XX_CONTROL_PADCONF_LCD_DATA0_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA1_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA2_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA3_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA4_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA5_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA6_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA7_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA8_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA9_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT  | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA10_OFFSET,OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA11_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT| AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA12_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT| AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA13_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT| AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA14_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT | AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_LCD_DATA15_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT| AM33XX_PULL_DISA},
	{AM33XX_CONTROL_PADCONF_GPMC_AD15_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD14_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD13_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD12_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD11_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD10_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD9_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_GPMC_AD8_OFFSET, OMAP_MUX_MODE1 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_LCD_VSYNC_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_LCD_HSYNC_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_LCD_PCLK_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT},
	{AM33XX_CONTROL_PADCONF_LCD_AC_BIAS_EN_OFFSET, OMAP_MUX_MODE0 | AM33XX_PIN_OUTPUT},
        {-1},
};

static struct module_pin_mux  lcd_backlight[] = 
{
#if defined(CONFIG_CORNERSTONE)
	{OFFSET(spi0_cs1), (MODE(2) | PULLUP_EN)},	//LCD_BL_PWM
	{OFFSET(mcasp0_fsr), (MODE(7) | PULLUP_EN)},	//LCD_LED_EN
#elif defined(CONFIG_PRISM)
        {OFFSET(ecap0_in_pwm0_out), (MODE(0) | PULLUP_EN)},	//LCD_BL_PWM
#endif        
	{-1},	
};

extern void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux);

static void enbale_bl_chip()
{
#ifdef CONFIG_CORNERSTONE
    writel(1<<19,SOC_GPIO_3_REGS+0x194); //set output data
#endif    
}

static void disable_bl_chip()
{    
#ifdef CONFIG_CORNERSTONE
    writel(1<<19,SOC_GPIO_3_REGS+0x190); //clear output data
#endif   
}

static void enable_backlight_pin()
{
    enbale_bl_chip();
    
#if defined(CONFIG_CORNERSTONE)
    //ECAP1
    writew(readw(SOC_ECAP_1_REGS+0x2a)|0x200,SOC_ECAP_1_REGS+0x2a);    
    writel(0,SOC_ECAP_1_REGS+0x0);
    writew(readw(SOC_ECAP_1_REGS+0x2a)&(~0x400),SOC_ECAP_1_REGS+0x2a); 
    writew(readw(SOC_ECAP_1_REGS+0x2a)|0x10,SOC_ECAP_1_REGS+0x2a);    
    writel(periodCnt,SOC_ECAP_1_REGS+0x8);  
    writel(periodCnt*60/100,SOC_ECAP_1_REGS+0xc); //default 60%    
#elif defined(CONFIG_PRISM)    
    //ECAP0
    writew(readw(SOC_ECAP_0_REGS+0x2a)|0x200,SOC_ECAP_0_REGS+0x2a);    
    writel(0,SOC_ECAP_0_REGS+0x0);
    writew(readw(SOC_ECAP_0_REGS+0x2a)&(~0x400),SOC_ECAP_0_REGS+0x2a); 
    writew(readw(SOC_ECAP_0_REGS+0x2a)|0x10,SOC_ECAP_0_REGS+0x2a);    
    writel(periodCnt,SOC_ECAP_0_REGS+0x8);  
    writel(periodCnt*60/100,SOC_ECAP_0_REGS+0xc); //default 60%
#endif
    
}

static void disable_backlight_pin()
{  
    disable_bl_chip();
    
#if defined(CONFIG_CORNERSTONE)
    //ECAP1
    writew(readw(SOC_ECAP_1_REGS+0x2a)|0x200,SOC_ECAP_1_REGS+0x2a);    
    writel(0,SOC_ECAP_1_REGS+0x0);
    writew(readw(SOC_ECAP_1_REGS+0x2a)&(~0x400),SOC_ECAP_1_REGS+0x2a); 
    writew(readw(SOC_ECAP_1_REGS+0x2a)|0x10,SOC_ECAP_1_REGS+0x2a);    
    writel(periodCnt,SOC_ECAP_1_REGS+0x8);  
    writel(0,SOC_ECAP_1_REGS+0xc); 
#elif defined(CONFIG_PRISM)    
    //ECAP0
    writew(readw(SOC_ECAP_0_REGS+0x2a)|0x200,SOC_ECAP_0_REGS+0x2a);    
    writel(0,SOC_ECAP_0_REGS+0x0);
    writew(readw(SOC_ECAP_0_REGS+0x2a)&(~0x400),SOC_ECAP_0_REGS+0x2a); 
    writew(readw(SOC_ECAP_0_REGS+0x2a)|0x10,SOC_ECAP_0_REGS+0x2a);    
    writel(periodCnt,SOC_ECAP_0_REGS+0x8);  
    writel(0,SOC_ECAP_0_REGS+0xc);
#endif
    
}

void lcd_pin_init()
{  
    configure_module_pin_mux(lcdc_pin_mux);
    //setup_pin_mux(lcdc_pin_mux, sizeof(lcdc_pin_mux)/sizeof(lcdc_pin_mux[0]));
    configure_module_pin_mux(lcd_backlight);
    
#ifdef CONFIG_CORNERSTONE    
    writel(readl(SOC_GPIO_3_REGS+0x134) & ~(1<<19),SOC_GPIO_3_REGS+0x134); //lcd bl en dir output
#endif

}

/* Disable the Raster Engine of the LCD Controller */
static void lcd_disable_raster()
{
	unsigned int reg;
	unsigned int loop_cnt = 0;
	unsigned int stat;
	unsigned int i = 0;

	reg = lcdc_read(LCD_RASTER_CTRL_REG);
	if (reg & LCD_RASTER_ENABLE)
		lcdc_write(reg & ~LCD_RASTER_ENABLE, LCD_RASTER_CTRL_REG);
}


static void lcd_reset()
{
	/* Disable the Raster if previously Enabled */
	lcd_disable_raster();

	/* DMA has to be disabled */
	lcdc_write(0, LCD_DMA_CTRL_REG);
	lcdc_write(0, LCD_RASTER_CTRL_REG);

	lcdc_write(0, LCD_INT_ENABLE_SET_REG);
	/* Write 1 to reset */
	lcdc_write(LCD_CLK_MAIN_RESET, LCD_CLK_RESET_REG);
	lcdc_write(0, LCD_CLK_RESET_REG);
}

static void lcd_calc_clk_divider()
{
	unsigned int lcd_clk, div;

	lcd_clk = 300000000;
	div = lcd_clk / 30000000;

	/* Configure the LCD clock divisor. */
	lcdc_write(LCD_CLK_DIVISOR(div) |(LCD_RASTER_MODE & 0x1), LCD_CTRL_REG);
	lcdc_write(LCD_V2_DMA_CLK_EN | LCD_V2_LIDD_CLK_EN |LCD_V2_CORE_CLK_EN, LCD_CLK_ENABLE_REG);
}

/* Configure the Burst Size and fifo threhold of DMA */
static int lcd_cfg_dma(int burst_size,  int fifo_th)
{
	unsigned int reg=0;

	//reg = lcdc_read(LCD_DMA_CTRL_REG) & 0x00000001;
	switch (burst_size) {
	case 1:
		reg |= LCD_DMA_BURST_SIZE(LCD_DMA_BURST_1);
		break;
	case 2:
		reg |= LCD_DMA_BURST_SIZE(LCD_DMA_BURST_2);
		break;
	case 4:
		reg |= LCD_DMA_BURST_SIZE(LCD_DMA_BURST_4);
		break;
	case 8:
		reg |= LCD_DMA_BURST_SIZE(LCD_DMA_BURST_8);
		break;
	case 16:
		reg |= LCD_DMA_BURST_SIZE(LCD_DMA_BURST_16);
		break;
	default:
		return -1;
	}

	reg |= (fifo_th << 8);

	lcdc_write(reg, LCD_DMA_CTRL_REG);

	return 0;
}

static void lcd_cfg_ac_bias(int period, int transitions_per_int)
{
	unsigned int reg;

	/* Set the AC Bias Period and Number of Transisitons per Interrupt */
	reg = lcdc_read(LCD_RASTER_TIMING_2_REG) & 0xFFF00000;
	reg |= LCD_AC_BIAS_FREQUENCY(period) |LCD_AC_BIAS_TRANSITIONS_PER_INT(transitions_per_int);
	lcdc_write(reg, LCD_RASTER_TIMING_2_REG);
}

static void lcd_cfg_horizontal_sync(int back_porch, int pulse_width,int front_porch)
{
	unsigned int reg;

	reg = lcdc_read(LCD_RASTER_TIMING_0_REG) & 0xf;
	reg |= ((back_porch & 0xff) << 24)
	    | ((front_porch & 0xff) << 16)
	    | ((pulse_width & 0x3f) << 10);
	lcdc_write(reg, LCD_RASTER_TIMING_0_REG);
}

static void lcd_cfg_vertical_sync(int back_porch, int pulse_width,int front_porch)
{
	unsigned int reg;

	reg = lcdc_read(LCD_RASTER_TIMING_1_REG) & 0x3ff;
	reg |= ((back_porch & 0xff) << 24)
	    | ((front_porch & 0xff) << 16)
	    | ((pulse_width & 0x3f) << 10);
	lcdc_write(reg, LCD_RASTER_TIMING_1_REG);
}

static int lcd_cfg_display()
{
	unsigned int reg;
	unsigned int reg_int;
        enum panel_shade panel_shade = COLOR_ACTIVE;	

	reg = lcdc_read(LCD_RASTER_CTRL_REG) & ~(LCD_TFT_MODE |
						LCD_MONO_8BIT_MODE |
						LCD_MONOCHROME_MODE);

	switch (panel_shade) {
	case MONOCHROME:
		reg |= LCD_MONOCHROME_MODE;
		#if 0
		if (cfg->mono_8bit_mode)
			reg |= LCD_MONO_8BIT_MODE;
		#endif	
		break;
	case COLOR_ACTIVE:
		reg |= LCD_TFT_MODE;
		#if 0
		if (cfg->tft_alt_mode)
			reg |= LCD_TFT_ALT_ENABLE;
		#endif	
		break;

	case COLOR_PASSIVE:
	        #if 0
		if (cfg->stn_565_mode)
			reg |= LCD_STN_565_ENABLE;
		#endif	
		break;

	default:
		return -1;
	}

	/* enable additional interrupts here */
        reg_int = lcdc_read(LCD_INT_ENABLE_SET_REG) | LCD_V2_UNDERFLOW_INT_ENA;
        lcdc_write(reg_int, LCD_INT_ENABLE_SET_REG);
	
	lcdc_write(reg, LCD_RASTER_CTRL_REG);

	reg = lcdc_read(LCD_RASTER_TIMING_2_REG);

	reg |= LCD_SYNC_CTRL;

	reg &= ~LCD_SYNC_EDGE;
	
	reg |= LCD_INVERT_LINE_CLOCK;
	
	reg |= LCD_INVERT_FRAME_CLOCK;	

	lcdc_write(reg, LCD_RASTER_TIMING_2_REG);

	return 0;
}

static int lcd_cfg_frame_buffer(unsigned int width, unsigned int height, unsigned int bpp)
{
	unsigned int reg;
	
        /*
         * 0x7F in bits 4..10 gives max horizontal resolution = 2048
         * pixels.
         */
        width &= 0x7f0;	

	reg = lcdc_read(LCD_RASTER_TIMING_0_REG);
	reg &= 0xfffffc00;

	width = (width >> 4) - 1;
	reg |= ((width & 0x3f) << 4) | ((width & 0x40) >> 3);

	lcdc_write(reg, LCD_RASTER_TIMING_0_REG);

	/* Set the Panel Height */
	/* Set bits 9:0 of Lines Per Pixel */
	reg = lcdc_read(LCD_RASTER_TIMING_1_REG);
	reg = ((height - 1) & 0x3ff) | (reg & 0xfffffc00);
	lcdc_write(reg, LCD_RASTER_TIMING_1_REG);

	reg = lcdc_read(LCD_RASTER_TIMING_2_REG);
	reg |= ((height - 1) & 0x400) << 16;
	lcdc_write(reg, LCD_RASTER_TIMING_2_REG);

	/* Set the Raster Order of the Frame Buffer */
	reg = lcdc_read(LCD_RASTER_CTRL_REG) & ~(1 << 8);

	if (bpp == 24)
		reg |= (LCD_TFT_MODE | LCD_V2_TFT_24BPP_MODE);
	else if (bpp == 32)
		reg |= (LCD_TFT_MODE | LCD_V2_TFT_24BPP_MODE
				| LCD_V2_TFT_24BPP_UNPACK);

	lcdc_write(reg, LCD_RASTER_CTRL_REG);

	return 0;
}

/* Enable the Raster Engine of the LCD Controller */
static inline void lcd_enable_raster(void)
{
	unsigned int reg;

	/* Put LCDC in reset for several cycles */

	lcdc_write(LCD_CLK_MAIN_RESET, LCD_CLK_RESET_REG);

	udelay(1000);

	/* Bring LCDC out of reset */
	lcdc_write(0, LCD_CLK_RESET_REG);

	udelay(1000);

	/* Above reset sequence doesnot reset register context */
	reg = lcdc_read(LCD_RASTER_CTRL_REG);
	if (!(reg & LCD_RASTER_ENABLE))
	{
		lcdc_write(reg | LCD_RASTER_ENABLE, LCD_RASTER_CTRL_REG);
	}	
}

int lcdc_Init()
{
	unsigned int bpp;
	int ret = 0;
	unsigned short palette[16];
	unsigned int tmp;

	lcd_pin_init();
	
	lcd_reset();

	/* Calculate the divider */
	lcd_calc_clk_divider();

	lcdc_write((lcdc_read(LCD_RASTER_TIMING_2_REG) |LCD_INVERT_PIXEL_CLOCK), LCD_RASTER_TIMING_2_REG);

	/* Configure the DMA burst size and fifo threshold. */
	ret = lcd_cfg_dma(16, 0);
	if (ret < 0)
		return ret;

	/* Configure the AC bias properties. */
	lcd_cfg_ac_bias(255, 0);

	/* Configure the vertical and horizontal sync properties. */
	lcd_cfg_vertical_sync(45,4,0);

	lcd_cfg_horizontal_sync(128, 4, 0);


	/* Configure for disply */
	ret = lcd_cfg_display();

	if (ret < 0)
		return ret;

	bpp = 16;	
	ret = lcd_cfg_frame_buffer(LCD_X_SIZE,LCD_Y_SIZE, bpp);
	if (ret < 0)
		return ret;

	/* Configure FDD */
	lcdc_write((lcdc_read(LCD_RASTER_CTRL_REG) & 0xfff00fff) |(255 << 12), LCD_RASTER_CTRL_REG);
//--------------------------------------------------------------------------
        palette[0] = 0x4000;
        memset(&palette[1],0,30);        
				
        lcdc_write(0, LCD_INT_ENABLE_SET_REG);    
        //------------------------------
        tmp = lcdc_read(LCD_RASTER_CTRL_REG);
        tmp &= ~(3 << 20);
        tmp |= 1 << 20;
        lcdc_write(tmp, LCD_RASTER_CTRL_REG);    
        lcdc_write((unsigned int)&palette[0],LCD_DMA_FRM_BUF_BASE_ADDR_0_REG);
        lcdc_write(((unsigned int)&palette[0])+31,LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG);    
        lcdc_write(0,LCD_DMA_FRM_BUF_BASE_ADDR_1_REG);
        lcdc_write(0,LCD_DMA_FRM_BUF_CEILING_ADDR_1_REG);       
        lcd_enable_raster();
        while(1)
        {
             tmp = 0;
             tmp = lcdc_read(LCD_RAW_STAT_REG);   
             if(0 != tmp)
             {  
                 lcdc_write(tmp, LCD_RAW_STAT_REG);
                 if(tmp & LCD_PL_LOAD_DONE)
                 {     
                      lcd_disable_raster();                  
                      lcdc_write(tmp, LCD_RAW_STAT_REG); 
                      break;
                 }
                  lcdc_write(0, LCD_END_OF_INT_IND_REG);
             }
        }    
        
        //-----------------------------  
        tmp = lcdc_read(LCD_RASTER_CTRL_REG);
        tmp &= ~(3 << 20);
        tmp |= 2 << 20;
        lcdc_write(tmp, LCD_RASTER_CTRL_REG);
        
        lcdc_write((unsigned int)&framebuf[0],LCD_DMA_FRM_BUF_BASE_ADDR_0_REG);
        lcdc_write((unsigned int)&framebuf[0]+2*LCD_X_SIZE*LCD_Y_SIZE-1,LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG);
        lcdc_write(0,LCD_DMA_FRM_BUF_BASE_ADDR_1_REG);
        lcdc_write(0,LCD_DMA_FRM_BUF_CEILING_ADDR_1_REG);
        lcd_enable_raster(); 

#if 0        
        printf("======================\n");
        printf("LCD_CLK_ENABLE_REG=0x%x\n",lcdc_read(LCD_CLK_ENABLE_REG));
        printf("LCD_DMA_CTRL_REG=0x%x\n",lcdc_read(LCD_DMA_CTRL_REG));
        printf("LCD_RASTER_TIMING_0_REG=0x%x\n",lcdc_read(LCD_RASTER_TIMING_0_REG));
        printf("LCD_RASTER_TIMING_1_REG=0x%x\n",lcdc_read(LCD_RASTER_TIMING_1_REG));    
        printf("LCD_RASTER_TIMING_2_REG=0x%x\n",lcdc_read(LCD_RASTER_TIMING_2_REG));
        printf("LCD_INT_ENABLE_SET_REG=0x%x\n",lcdc_read(LCD_INT_ENABLE_SET_REG));
        printf("LCD_DMA_FRM_BUF_BASE_ADDR_0_REG=0x%x\n",lcdc_read(LCD_DMA_FRM_BUF_BASE_ADDR_0_REG));
        printf("LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG=0x%x\n",lcdc_read(LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG)); 
        printf("LCD_RASTER_CTRL_REG=0x%x\n",lcdc_read(LCD_RASTER_CTRL_REG));     
        printf("LCD_CTRL_REG=0x%x\n",lcdc_read(LCD_CTRL_REG));     
        printf("======================\n");      
#endif

}

static void do_lcd_update()
{
    unsigned int tmp;

    lcdc_write((unsigned int)&framebuf[0],LCD_DMA_FRM_BUF_BASE_ADDR_0_REG);
    lcdc_write((unsigned int)&framebuf[0]+2*LCD_X_SIZE*LCD_Y_SIZE-1,LCD_DMA_FRM_BUF_CEILING_ADDR_0_REG);                  
    
    while(1)
    {   
         tmp = 0;
         tmp = lcdc_read(LCD_RAW_STAT_REG);   
         if(0 != tmp)
         {  
             lcdc_write(tmp, LCD_RAW_STAT_REG);
             if(tmp & LCD_END_OF_FRAME0)
             {
                    break;
             }
             lcdc_write(0, LCD_END_OF_INT_IND_REG);
         }
    }
}

void FlushFrameBufCache()
{
    do_lcd_update();
}

void clear_screen(unsigned int color)
{  
    int tmp = 0;
    
    for(tmp=0; tmp<(LCD_X_SIZE*LCD_Y_SIZE); tmp++)
    {
        framebuf[tmp] = color & 0xFFFF;
    }
    do_lcd_update();    
}

void lcdc_bl_on()
{
    enable_backlight_pin();
}

void lcdc_bl_off()
{
    disable_backlight_pin();
}

#ifdef _DEBUG_

void lcdc_bmp(int argc, char *argv[])
{
    printf("gsop doesn't support\n");
}

#define MAX_RGB (256)

/**
* fill  colors accoring to column
*/
void fill_column_colors(unsigned short colors[],int cnr,int column)
{
    int m=0;
    int n=0;
    int idx=0;
    int cidx=0;
    
    memset(framebuf,0,LCD_X_SIZE*LCD_Y_SIZE);    
    if(column ==0 || (column > LCD_X_SIZE/cnr))
    {
        column = LCD_X_SIZE/cnr;
    }  
    
    for(m=0;m < LCD_Y_SIZE;m++)
    {
        for(n =0;n<LCD_X_SIZE;n++)
        {
            idx = m*LCD_X_SIZE +n;        
            if(n!=0 && (n % column) == 0)
            {
                cidx++;
            }
            if((cidx % cnr) == 0)
            {
                cidx =0;      
            }
            framebuf[idx]=colors[cidx] & 0xFFFF;        
        }
        cidx =0;
    }
}

/**
*  fill colors accoring to row
*/
void fill_row_colors(unsigned short colors[],int cnr,int row)
{
    int m=0;
    int n=0;
    int idx=0;
    int cidx=0;
    
    memset(framebuf,0,LCD_X_SIZE*LCD_Y_SIZE);
    
    if(row ==0 || (row > LCD_Y_SIZE/cnr))
    {
        row = LCD_Y_SIZE/cnr;
    }
    
    for(m=0;m < LCD_Y_SIZE;m++)
    {
        if(m!=0 && (m % row) == 0)
        {
            cidx++;
        }    
        if((cidx % cnr) == 0)
        {
            cidx =0;
        }    
        for(n =0;n<LCD_X_SIZE;n++)
        {
            idx = m*LCD_X_SIZE +n;        
            framebuf[idx]=colors[cidx] & 0xFFFF;        
        }
    }
}

/**
*  ring copy 'ccount' src colors to dest 'index' 
*  indicate the begin postion at src.  
*/
int getcolors(unsigned short dest[],unsigned short src[],int index,int ccount)
{
    int idx =0;
    int j=0;
    idx =index % ccount;
    
    for(j=0;j<ccount;j++,idx++)
    {
        if(idx %ccount ==0)
        {
            idx =0;
        }
        dest[j]=src[idx];
    }
}

/**
* test colors accoring column
*/
void test_column_colors(unsigned short src[],int ccount,int column)
{
    unsigned short colors[MAX_RGB]={0};
    int i=1;
    
    while(i> 0)
    {    
        getcolors(colors,src,i,ccount);
        fill_column_colors(colors,ccount,column);
        do_lcd_update();
        i--;
    }		
}

/**
* test rgb colors accoring row
*/
void test_row_colors(unsigned short src[],int ccount,int row)
{
    //unsigned short rgb[MAX_RGB]={0xf800,0x7e0,0x1f};
    unsigned short colors[MAX_RGB]={0};
    int i=1;
    
    while(i> 0)
    {    
        getcolors(colors,src,i,ccount);
        fill_row_colors(colors,ccount,row);
        do_lcd_update();
        i--;
    }
}


void test_lcd_color()
{
    int i=0;
    unsigned short color = 0;
    unsigned short rgb[3]={0xf800,0x7e0,0x1f};
    unsigned short bw[2]={0xffff,0x0};
    for(i=0; i<3; i++)
    {
        if(i == 0)
        {
             color = 0xf800;  // 16 bit red
        }
        else if(i == 1)
        {
             color = 0x7e0;  // 16 bit green
        }
        else if(i == 2)
        {
             color = 0x1f;  // 16 bit blue
        }        
        
        clear_screen(color);
        mdelay(3000);  //3s delay        
    }

    test_row_colors(rgb,3,48);
    mdelay(3000);
    test_column_colors(rgb,3,64);
    mdelay(3000);
    test_row_colors(bw,2,48);
    mdelay(3000);
    test_column_colors(bw,2,64);
}

#endif

