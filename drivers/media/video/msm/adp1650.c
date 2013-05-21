/*
 * ADP1650 LED Flash Driver
 *
 * Copyright 2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/leds.h>
/*
 * ADP1650 Registers
 */
#define ADP1650_REG_VERSION		0x00
#define ADP1650_REG_TIMER_IOCFG		0x02
#define ADP1650_REG_CURRENT_SET		0x03
#define ADP1650_REG_OUTPUT_MODE		0x04
#define ADP1650_REG_FAULT		0x05
#define ADP1650_REG_CONTROL		0x06
#define ADP1650_REG_AD_MODE		0x07
#define ADP1650_REG_ADC			0x08
#define ADP1650_REG_BATT_LOW		0x09

/* ADP1650_REG_TIMER_IOCFG Bits and Masks */
#define ADP1650_IOCFG_IO2_HIGH_IMP	(0 << 6) /* High Impedance */
#define ADP1650_IOCFG_IO2_IND_LED	(1 << 6) /* Indicator LED */
#define ADP1650_IOCFG_IO2_TXMASK2	(2 << 6) /* TxMASK2 operation mode */
#define ADP1650_IOCFG_IO2_AIN		(3 << 6) /* ADC analog input */
#define ADP1650_IOCFG_IO1_HIGH_IMP	(0 << 4) /* High Impedance */
#define ADP1650_IOCFG_IO1_TORCH		(1 << 4) /* Torch mode */
#define ADP1650_IOCFG_IO1_TXMASK1	(2 << 4) /* TxMASK1 operation mode */
#define ADP1650_FL_TIMER_ms(x)		((((x) - 100) / 100) & 0xF) /* Timer */

/* ADP1650_REG_CURRENT_SET Bits and Masks  */
#define ADP1650_I_FL_mA(x)		((((x) - 300) / 50) << 3)
#define ADP1650_I_TOR_mA(x)		((((x) - 25) / 25) & 0x7)

/* ADP1650_REG_OUTPUT_MODE Bits and Masks  */
#define ADP1650_IL_PEAK_1A75		(0 << 6)
#define ADP1650_IL_PEAK_2A25		(1 << 6)
#define ADP1650_IL_PEAK_2A75		(2 << 6)
#define ADP1650_IL_PEAK_3A00		(3 << 6)
#define ADP1650_STR_LV_EDGE		(0 << 5)
#define ADP1650_STR_LV_LEVEL		(1 << 5)
#define ADP1650_FREQ_FB_EN		(1 << 4)
#define ADP1650_OUTPUT_EN		(1 << 3)
#define ADP1650_STR_MODE_SW		(0 << 2)
#define ADP1650_STR_MODE_HW		(1 << 2)
#define ADP1650_STR_MODE_STBY		(0 << 0)
#define ADP1650_LED_MODE_VOUT		(1 << 0)
#define ADP1650_LED_MODE_ASSIST_LIGHT	(2 << 0)
#define ADP1650_LED_MODE_FLASH		(3 << 0)

/* ADP1650_REG_FAULT Bits and Masks  */
#define ADP1650_FL_OVP			(1 << 7)
#define ADP1650_FL_SC			(1 << 6)
#define ADP1650_FL_OT			(1 << 5)
#define ADP1650_FL_TO			(1 << 4)
#define ADP1650_FL_TX1			(1 << 3)
#define ADP1650_FL_IO2			(1 << 2)
#define ADP1650_FL_IL			(1 << 1)
#define ADP1650_FL_IDC			(1 << 0)

/* ADP1650_REG_CONTROL Bits and Masks  */
#define ADP1650_I_TX2_mA(x)		((((x) - 100) / 50) << 4)
#define ADP1650_I_TX1_mA(x)		((((x) - 100) / 50) & 0xF)

/* ADP1650_REG_AD_MODE Bits and Masks  */
#define ADP1650_DYN_OVP_EN			(1 << 7)
#define ADP1650_SW_LO_1MHz5			(1 << 6)
#define ADP1650_STR_POL_ACTIVE_HIGH		(1 << 5)
#define ADP1650_I_ILED_2mA75			(0 << 4)
#define ADP1650_I_ILED_5mA50			(1 << 4)
#define ADP1650_I_ILED_8mA25			(2 << 4)
#define ADP1650_I_ILED_11mA00			(3 << 4)
#define ADP1650_IL_DC_1A50			(0 << 1)
#define ADP1650_IL_DC_1A75			(1 << 1)
#define ADP1650_IL_DC_2A00			(2 << 1)
#define ADP1650_IL_DC_2A25			(3 << 1)
#define ADP1650_IL_DC_EN			(1 << 0)

/* ADP1650_REG_ADC Bits and Masks  */
#define ADP1650_FL_VB_LO			(1 << 6)
#define ADP1650_ADC_VAL(x)			(((x) & 0x3C) >> 2)
#define ADP1650_ADC_DIS				(0 << 0)
#define ADP1650_ADC_LED_VF			(1 << 0)
#define ADP1650_ADC_DIE_TEMP			(2 << 0)
#define ADP1650_ADC_EXT_VOLT			(3 << 0)

/* ADP1650_REG_BATT_LOW Bits and Masks  */
#define ADP1650_CL_SOFT_EN			(1 << 7)
#define ADP1650_I_VB_LO_mA(x)			((((x) - 300) / 50) << 3)
#define ADP1650_V_VB_LO_DIS			(0 << 0)
#define ADP1650_V_VB_LO_3V30			(1 << 0)
#define ADP1650_V_VB_LO_3V35			(2 << 0)
#define ADP1650_V_VB_LO_3V40			(3 << 0)
#define ADP1650_V_VB_LO_3V45			(4 << 0)
#define ADP1650_V_VB_LO_3V50			(5 << 0)
#define ADP1650_V_VB_LO_3V55			(6 << 0)
#define ADP1650_V_VB_LO_3V60			(7 << 0)

/*
 * /sys/class/leds/adp1650/brightness values / mode steering
 */

#define FL_MODE_OFF			0 /* OFF */
#define FL_MODE_TORCH_25mA		1 /* SW trigged TORCH to FLASH */
#define FL_MODE_TORCH_50mA		2 /* TORCH Intensity XmA */
#define FL_MODE_TORCH_75mA		3
#define FL_MODE_TORCH_100mA		4
#define FL_MODE_TORCH_125mA		5
#define FL_MODE_TORCH_150mA		6
#define FL_MODE_TORCH_175mA		7
#define FL_MODE_TORCH_200mA		8
#define FL_MODE_TORCH_TRIG_EXT_25mA	9 /* HW/IO trigged TORCH to FLASH */
#define FL_MODE_TORCH_TRIG_EXT_50mA	10/* TORCH Intensity XmA */
#define FL_MODE_TORCH_TRIG_EXT_75mA	11
#define FL_MODE_TORCH_TRIG_EXT_100mA	12
#define FL_MODE_TORCH_TRIG_EXT_125mA	13
#define FL_MODE_TORCH_TRIG_EXT_150mA	14
#define FL_MODE_TORCH_TRIG_EXT_175mA	15
#define FL_MODE_TORCH_TRIG_EXT_200mA	16
#define FL_MODE_FLASH			254 /* SW triggered FLASH */
#define FL_MODE_FLASH_TRIG_EXT		255 /* HW/Strobe trigged FLASH */



struct i2c_client; /* forward declaration */
struct adp1650_leds_platform_data {
	unsigned char timer_iocfg;	/* See ADP1650_REG_TIMER_IOCFG Bits */
	unsigned char current_set;	/* See ADP1650_REG_CURRENT_SET Bits */
	unsigned char output_mode;	/* See ADP1650_REG_OUTPUT_MODE Bits */
	unsigned char control;		/* See ADP1650_REG_CONTROL Bits */
	unsigned char ad_mode;		/* See ADP1650_REG_AD_MODE Bits */
	unsigned char batt_low;		/* See ADP1650_REG_BATT_LOW Bits */

	/* system specific GPIO to control the ADP1650_EN pin,
	 * if not used set to -1
	 */
	int gpio_enable;

	/* system specific setup callback */
	int	(*setup)(struct i2c_client *client,
			 unsigned state);
};

struct i2c_client *adp1650_client;

struct adp1650_chip {
	struct i2c_client *client;
	struct led_classdev cdev;
	struct adp1650_leds_platform_data *pdata;
	unsigned char iocfg;
	unsigned char current_set;
	bool use_enable;
};

static struct adp1650_leds_platform_data ad1650_default_pdata = {
	.timer_iocfg =	ADP1650_IOCFG_IO2_HIGH_IMP |
			ADP1650_IOCFG_IO1_TORCH |
			ADP1650_FL_TIMER_ms(500),

	.current_set =	ADP1650_I_FL_mA(900) |
			ADP1650_I_TOR_mA(100),

	.output_mode =	ADP1650_IL_PEAK_1A75 |
			ADP1650_STR_LV_EDGE |
			ADP1650_FREQ_FB_EN |
			ADP1650_OUTPUT_EN |
			ADP1650_STR_MODE_HW |
			ADP1650_STR_MODE_STBY,

	.control =	ADP1650_I_TX2_mA(400) |
			ADP1650_I_TX1_mA(400),

	.ad_mode =	ADP1650_DYN_OVP_EN |
			ADP1650_STR_POL_ACTIVE_HIGH |
			ADP1650_I_ILED_2mA75 |
			ADP1650_IL_DC_1A50 |
			ADP1650_IL_DC_EN,

	.batt_low =	ADP1650_CL_SOFT_EN |
			ADP1650_I_VB_LO_mA(400) |
			ADP1650_V_VB_LO_3V50,

	.gpio_enable = 2,
};

static inline int adp1650_write(struct i2c_client *client, u8 reg, u8 value)
{
	int ret = i2c_smbus_write_byte_data(client, reg, value);
	if (ret < 0)
		dev_err(&client->dev, "i2c write failed\n");

	return ret;
}

static int adp1650_read(struct i2c_client *client, u8 reg, u8 *buf)
{
	int ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "i2c read failed\n");
		return ret;
	}

	*buf = ret;

	return 0;
}

static int adp1650_get_fault_status(struct i2c_client *client)
{
	unsigned char fault;
	int ret = adp1650_read(client, ADP1650_REG_FAULT, &fault);
	if (ret < 0)
		return ret;
	pr_err("%s: entry\n", __func__);

	pr_err("FAULT = %X:\n%s%s%s%s%s%s%s%s\n", fault,
		fault & ADP1650_FL_OVP ? "FL_OVP\n" : "",
		fault & ADP1650_FL_SC ? "FL_SC\n" : "",
		fault & ADP1650_FL_OT ? "FL_OT\n" : "",
		fault & ADP1650_FL_TO ? "FL_TO\n" : "",
		fault & ADP1650_FL_TX1 ? "FL_TX1\n" : "",
		fault & ADP1650_FL_IO2 ? "FL_IO2\n" : "",
		fault & ADP1650_FL_IL ? "FL_IL\n" : "",
		fault & ADP1650_FL_IDC ? "FL_IDC\n" : "");

	return fault;
}

static int adp1650_setup(struct i2c_client *client)
{
	struct adp1650_chip *chip = i2c_get_clientdata(client);
	const struct adp1650_leds_platform_data *pdata = chip->pdata;
	int ret;
	unsigned char fault;
	
	pr_err("%s: entry\n", __func__);
	ret = adp1650_write(client, ADP1650_REG_TIMER_IOCFG,
			    pdata->timer_iocfg);
	if (ret < 0)
		return ret;

	ret = adp1650_write(client, ADP1650_REG_CURRENT_SET,
			    pdata->current_set);
	if (ret < 0)
		return ret;

	ret = adp1650_read(client, ADP1650_REG_CURRENT_SET, &fault);
	pr_err("%s: entry :%d\n", __func__,fault);


	ret = adp1650_write(client, ADP1650_REG_OUTPUT_MODE,
			    pdata->output_mode);
	if (ret < 0)
		return ret;

	ret = adp1650_write(client, ADP1650_REG_CONTROL, pdata->control);
	if (ret < 0)
		return ret;

	ret = adp1650_write(client, ADP1650_REG_AD_MODE, pdata->ad_mode);
	if (ret < 0)
		return ret;

	ret = adp1650_write(client, ADP1650_REG_BATT_LOW, pdata->batt_low);
	if (ret < 0)
		return ret;

	chip->iocfg = pdata->timer_iocfg;
	chip->current_set = pdata->current_set;

	adp1650_get_fault_status(client); /* Clear Fault Register */

	return 0;
}

static int adp1650_led_mode_set(struct i2c_client *client, int mode)
{
	struct adp1650_chip *chip = i2c_get_clientdata(client);
	const struct adp1650_leds_platform_data *pdata = chip->pdata;
	int ret;
	unsigned char output, iocfg, current_set;
	pr_err("%s: entry\n", __func__);

#ifdef ADP1650_CHECK_AND_CLEAR_FAULT
	ret = adp1650_get_fault_status(client);
	if (ret < 0)
		return ret;
	else if (ret & (ADP1650_FL_OVP | ADP1650_FL_SC | ADP1650_FL_OT |
			ADP1650_FL_TO | ADP1650_FL_IL | ADP1650_FL_IDC))
	pr_err("critical fault status (0x%X)\n",ret);
#endif
	iocfg = chip->iocfg;
	current_set = chip->current_set;

	switch (mode) {
	case FL_MODE_OFF:
		output = pdata->output_mode &
			~(ADP1650_OUTPUT_EN | ADP1650_LED_MODE_FLASH);
		break;		
	case FL_MODE_TORCH_25mA:
		output = pdata->output_mode & ~(ADP1650_LED_MODE_FLASH);
		output |= ADP1650_OUTPUT_EN | ADP1650_LED_MODE_ASSIST_LIGHT;
		iocfg &= ~ADP1650_IOCFG_IO1_TORCH;
		current_set = (current_set & 0xF8) |
			(mode - FL_MODE_TORCH_25mA);
		break;		
	case FL_MODE_TORCH_TRIG_EXT_25mA:
		output = pdata->output_mode & ~(ADP1650_LED_MODE_FLASH);
		output |= ADP1650_OUTPUT_EN | ADP1650_STR_MODE_STBY;
		iocfg |= ADP1650_IOCFG_IO1_TORCH;
		current_set = (current_set & 0xF8) |
			(mode - FL_MODE_TORCH_TRIG_EXT_25mA);
		break;
	case FL_MODE_FLASH:
		output = pdata->output_mode &
			~(ADP1650_LED_MODE_FLASH | ADP1650_STR_MODE_HW);
		output |= ADP1650_OUTPUT_EN | ADP1650_LED_MODE_FLASH |
			ADP1650_STR_MODE_SW;
		break;
	case FL_MODE_FLASH_TRIG_EXT:
		output = pdata->output_mode & ~(ADP1650_LED_MODE_FLASH);
		output |= ADP1650_OUTPUT_EN | ADP1650_LED_MODE_FLASH |
			ADP1650_STR_MODE_HW;
		break;

	default:
		return -EINVAL;
	}

	if (current_set != chip->current_set) {
		ret = adp1650_write(client, ADP1650_REG_CURRENT_SET,
				    current_set);
		if (ret < 0)
			return ret;
		chip->current_set = current_set;
	}

	if (iocfg != chip->iocfg) {
		ret = adp1650_write(client, ADP1650_REG_TIMER_IOCFG, iocfg);
		if (ret < 0)
			return ret;
		chip->iocfg = iocfg;
	}

	ret = adp1650_write(client, ADP1650_REG_OUTPUT_MODE, output);
	if (ret < 0)
		return ret;

	return 0;
}

static void adp1650_brightness_set(struct led_classdev *led_cdev,
						enum led_brightness brightness)
{
	struct adp1650_chip *chip = container_of(led_cdev,
						 struct adp1650_chip, cdev);
	pr_err("%s: entry\n", __func__);
	adp1650_led_mode_set(chip->client, brightness);
}
int ADP1650_disable(void)
{
	int rc=0;
        pr_err("%s: entry\n", __func__);	
	rc = adp1650_write(adp1650_client, 0x02,0x00);			
	rc = adp1650_write(adp1650_client, 0x04,0x24);	
	rc = gpio_request(18, "adp1650");	
	rc = gpio_direction_output(18, 0);
	pr_err("gpio 18 set 0\n");		
//	rc = gpio_request(19, "adp1650");	
//	rc = gpio_direction_output(19, 0);
//	pr_err("gpio 19 set direction OK\n");		
	return rc;
}
int ADP1650_assistmode_enable(void)
{
	int rc=0;
	unsigned char buf;	
       pr_err("%s: entry\n", __func__);
	rc = adp1650_write(adp1650_client, 0x02,0x04);
	rc = adp1650_write(adp1650_client, 0x03,0x53);	
	rc = adp1650_write(adp1650_client, 0x04,0x0E);	
	rc = adp1650_write(adp1650_client, 0x07,0x21);//24
	rc = adp1650_read(adp1650_client, 0x05,&buf);
	pr_err("read 0x05 value is %x\n",buf);	
	return rc;
}
int ADP1650_flashmode_enable(void)
{
	int rc=0;
	unsigned char buf;
       pr_err("%s: entry\n", __func__);	

	rc = adp1650_write(adp1650_client, 0x02,0x04);
	rc = adp1650_write(adp1650_client, 0x03,0x53);	
	rc = adp1650_read(adp1650_client, 0x05,&buf);
	pr_err("read 0x05 value is %x\n",buf);	
       rc = gpio_request(18, "adp1650");
	rc = gpio_direction_output(18, 1);
	pr_err("gpio 18 set direction 1\n");	
	if (rc) {
	pr_err("gpio 18 set direction failed\n");
	}	
	return rc;
}

static int __devinit adp1650_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
			{
	struct adp1650_chip *chip;
	//unsigned char buf;
	int ret;
	pr_err("%s: entry\n", __func__);

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) 
			{
		pr_err("%s i2c byte data not supported\n",__func__);
		return -EIO;
	}
	chip = kzalloc(sizeof(struct adp1650_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	if (!client->dev.platform_data) {
		pr_err("%s pdata is not available, using default\n",__func__);		
			chip->pdata = &ad1650_default_pdata;
	} else {
		chip->pdata = client->dev.platform_data;
	}	

	chip->client = client;
	adp1650_client = client;
	i2c_set_clientdata(client, chip);

	if (chip->pdata->setup) {
		ret = chip->pdata->setup(client, true);
		if (ret < 0) {
			dev_err(&client->dev, "setup callback failed!\n");
			//goto err_free_mem;
		}
	}

	ret = gpio_is_valid(chip->pdata->gpio_enable);
	if (ret) {
		ret = gpio_request(chip->pdata->gpio_enable, id->name);
		if (ret) {
			dev_err(&client->dev, "gpio %d request failed\n",
					chip->pdata->gpio_enable);
			//goto err_setup;
		}
		chip->use_enable = true;

		ret = gpio_direction_output(chip->pdata->gpio_enable, 1);
		if (ret) {
			dev_err(&client->dev, "gpio %d set direction failed\n",
						chip->pdata->gpio_enable);
			goto err_release_gpio;
		}
	}

	ret = adp1650_setup(chip->client);
	if (ret < 0) {
		dev_err(&client->dev, "device setup failed %d\n", ret);
		goto err_release_gpio;
	}
	ADP1650_disable();	
       //ADP1650_flashmode_enable();	
	
	chip->cdev.name = client->name;
	chip->cdev.brightness = FL_MODE_OFF;
	chip->cdev.brightness_set = adp1650_brightness_set;

	ret = led_classdev_register(&client->dev, &chip->cdev);
	if (ret < 0) {
		dev_err(&client->dev, "failed to register led");
		goto err_release_gpio;
	}

	return 0;

err_release_gpio:
	if (chip->use_enable)
		gpio_free(chip->pdata->gpio_enable);


	if (chip->pdata->setup)
		chip->pdata->setup(client, true);

	kfree(chip);

	return ret;
}

static int __devexit adp1650_remove(struct i2c_client *client)
{
	struct adp1650_chip *chip = i2c_get_clientdata(client);

	led_classdev_unregister(&chip->cdev);
	adp1650_led_mode_set(client, FL_MODE_OFF);

	if (chip->use_enable)
		gpio_free(chip->pdata->gpio_enable);

	if (chip->pdata->setup)
		chip->pdata->setup(client, false);

	kfree(chip);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int adp1650_suspend(struct device *dev)
{
	struct adp1650_chip *chip = dev_get_drvdata(dev);
	int ret;


	/*if (chip->use_enable) {
		pr_err("lijing:set gpio 0\n");
		gpio_set_value_cansleep(chip->pdata->gpio_enable, 0);
		}
	else*/
		adp1650_led_mode_set(chip->client, FL_MODE_OFF);

	if (chip->pdata->setup) {
		ret = chip->pdata->setup(chip->client, false);
		if (ret) {
			pr_err("setup failed");
			return ret;
		}
	}
	return 0;
}

static int adp1650_resume(struct device *dev)
{
	struct adp1650_chip *chip = dev_get_drvdata(dev);
	int ret;

	if (chip->pdata->setup) {
		ret = chip->pdata->setup(chip->client, true);
		if (ret) {
			pr_err("setup failed\n");
			return ret;
		}
	}

	adp1650_setup(chip->client);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(adp1650_pm_ops, adp1650_suspend, adp1650_resume);

static const struct i2c_device_id adp1650_id[] = {
	{"adp1650", 0},
	{ },
};
MODULE_DEVICE_TABLE(i2c, adp1650_id);

static struct i2c_driver adp1650_driver = {
	.driver = {
		.name = KBUILD_MODNAME,
		.pm = &adp1650_pm_ops,
		.owner = THIS_MODULE,
	},
	.probe = adp1650_probe,
	.remove = __devexit_p(adp1650_remove),
	.id_table = adp1650_id,
};

static int __init adp1650_init(void)
{
	pr_err("%s: entry\n", __func__);
	return i2c_add_driver(&adp1650_driver);
}
module_init(adp1650_init);
	
static void __exit adp1650_exit(void)
{
	i2c_del_driver(&adp1650_driver);
}
module_exit(adp1650_exit);

MODULE_AUTHOR("Michael Hennerich <michael.hennerich@analog.com>");
MODULE_DESCRIPTION("ADP1650 LED Flash Driver");
MODULE_LICENSE("GPL v2");
