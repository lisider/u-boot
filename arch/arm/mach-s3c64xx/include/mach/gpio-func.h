#ifndef __GPIO_FUNC_S3C6410__
#define __GPIO_FUNC_S3C6410__

int s3c64xx_gpio_set_value(unsigned gpio, int value); 
int s3c64xx_gpio_get_value(unsigned gpio); 
int s3c64xx_gpio_set_cfg(unsigned gpio, int cfg); 
int s3c64xx_gpio_get_cfg(unsigned gpio); 
int s3c64xx_gpio_set_pull(unsigned gpio, int mode);

#define gpio_set_value(gpio,cfg)  s3c64xx_gpio_set_value(gpio,cfg)
#define gpio_get_value(gpio)       s3c64xx_gpio_get_value(gpio)
#define gpio_set_cfg(gpio,cfg)     s3c64xx_gpio_set_cfg(gpio,cfg)
#define gpio_get_cfg(gpio)          s3c64xx_gpio_get_value(gpio)
#define gpio_set_pull(gpio,mode) s3c64xx_gpio_set_pull(gpio,mode)

#endif /* __GPIO_FUNC_S3C6410__ */
