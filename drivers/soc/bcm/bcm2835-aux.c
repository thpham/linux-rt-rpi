/*
 * bcm2835-aux
 *
 * Copyright (C) 2015 Martin Sperl
 *
 * Author: Martin Sperl <kernel@martin.sperl.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/soc/bcm/bcm2835-aux.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(bcm2835aux_lock);

static struct platform_driver bcm2835aux_driver;

static int bcm2835aux_dev_match(struct device *dev, void *data)
{
	struct device_node *dn = data;

	return (dev->of_node == dn) ? 1 : 0;
}

static void *bcm2835aux_find_base(struct device *dev, const char *property)
{
	struct device *found = NULL;
	struct device_node *np;

	/* get the phandle of the device */
	np = of_parse_phandle(dev->of_node, property, 0);
	if (!np) {
		dev_err(dev, "missing property %s\n", property);
		return ERR_PTR(-ENODEV);
	}

	/* now find the device it points to */
	found = driver_find_device(&bcm2835aux_driver.driver, NULL,
				   np, bcm2835aux_dev_match);
	if (!found) {
		dev_err(dev, "device for phandle of %s not found\n",
			property);
		return ERR_PTR(-EPROBE_DEFER);
	}

	/* now we got the device, so return the pointer */
	return dev_get_drvdata(found);
}

static u32 bcm2835aux_find_mask(struct device *dev, const char *property)
{
	int err;
	u32 mask;

	err = of_property_read_u32_index(dev->of_node, property, 1, &mask);
	if (err) {
		dev_err(dev, "missing argument to %s: %d\n",
			property, err);
		return 0;
	}

	return mask;
}

static int bcm2835aux_bitset(struct device *dev, const char *property,
			     bool set)
{
	u32 v, mask;
	unsigned long flags;
	void __iomem *base;

	/* find the device */
	base = bcm2835aux_find_base(dev, property);
	if (IS_ERR(base))
		return PTR_ERR(base);

	/* and extract the mask */
	mask = bcm2835aux_find_mask(dev, property);
	if (!mask)
		return -ENOENT;

	spin_lock_irqsave(&bcm2835aux_lock, flags);

	v = readl(base);
	if (set)
		v |= mask;
	else
		v &= ~mask;

	writel(v, base);

	spin_unlock_irqrestore(&bcm2835aux_lock, flags);

	return 0;
}

int bcm2835aux_enable(struct device *dev, const char *property)
{
	return bcm2835aux_bitset(dev, property, true);
}
EXPORT_SYMBOL_GPL(bcm2835aux_enable);

int bcm2835aux_disable(struct device *dev, const char *property)
{
	return bcm2835aux_bitset(dev, property, false);
}
EXPORT_SYMBOL_GPL(bcm2835aux_disable);

static int bcm2835aux_probe(struct platform_device *pdev)
{
	struct resource *res;
	void __iomem *base;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOENT;

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	platform_set_drvdata(pdev, base);

	return 0;
}

static const struct of_device_id bcm2835aux_match[] = {
	{ .compatible = "brcm,bcm2835-aux", },
	{}
};
MODULE_DEVICE_TABLE(of, bcm2835aux_match);

static struct platform_driver bcm2835aux_driver = {
	.driver = {
		.name           = "bcm2835-aux",
		.of_match_table	= bcm2835aux_match,
	},
	.probe			= bcm2835aux_probe,
};
module_platform_driver(bcm2835aux_driver);

MODULE_DESCRIPTION("enable/disable driver for aux-spi1/spi2/uart1 on Broadcom BCM2835");
MODULE_AUTHOR("Martin Sperl <kernel@martin.sperl.org>");
MODULE_LICENSE("GPL v2");
