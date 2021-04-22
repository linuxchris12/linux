/*
 * ASoC driver for a generic I2S mic
 * connected to a Raspberry Pi
 *
 * Author:		Christoph Orth, <c.orth@ugreen.eu>
 * Based on work from:  Bjoern Biesenbach, <bjoern@bjoern-b.de>; Heiko Jehmlich <hje@jecons.de>
 * Copyright 2021
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <sound/soc.h>


SND_SOC_DAILINK_DEFS(generic_i2s_mic,
	DAILINK_COMP_ARRAY(COMP_CPU("bcm2708-i2s.0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("dmic-codec", "dmic-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("bcm2708-i2s.0")));

static struct snd_soc_dai_link snd_generic_i2s_mic_dai[] = {
{
	.name		= "I2SMic",
	.stream_name	= "I2SMic Hifi",
	.dai_fmt	= 0,
	SND_SOC_DAILINK_REG(generic_i2s_mic),
},
};

/* audio machine driver */
static struct snd_soc_card snd_generic_i2s_mic = {
	.name		= "snd_i2s_mic",
	.driver_name	= "snd_i2s_mic",
	.dai_link	= snd_generic_i2s_mic_dai,
	.owner		= THIS_MODULE,
	.num_links	= ARRAY_SIZE(snd_generic_i2s_mic_dai),
};

static int snd_generic_i2s_mic_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &snd_generic_i2s_mic;
	struct snd_soc_dai_link *dai = &snd_generic_i2s_mic_dai[0];
	struct device_node *i2s_node = pdev->dev.of_node;
	bool val = false;

	card->dev = &pdev->dev;

	val = of_property_read_bool(pdev->dev.of_node, "rpi,snd_soc_daifmt_i2s");
	if (val)
		dai->dai_fmt |= SND_SOC_DAIFMT_I2S;
	
	val = of_property_read_bool(pdev->dev.of_node, "rpi,snd_soc_daifmt_nb_nf");
	if (val)
		dai->dai_fmt |= SND_SOC_DAIFMT_NB_NF;
	
	val = of_property_read_bool(pdev->dev.of_node, "rpi,snd_soc_daifmt_cbm_cfm");
	if (val)
		dai->dai_fmt |= SND_SOC_DAIFMT_CBM_CFM;
	
	if (!dai) {
		dev_err(&pdev->dev, "DAI not found. Missing or Invalid\n");
		return -EINVAL;
	}

	i2s_node = of_parse_phandle(pdev->dev.of_node, "i2s-controller", 0);
	if (!i2s_node) {
		dev_err(&pdev->dev,
			"Property 'i2s-controller' missing or invalid\n");
		return -EINVAL;
	}

	dai->cpus->dai_name = NULL;
	dai->cpus->of_node = i2s_node;
	dai->platforms->name = NULL;
	dai->platforms->of_node = i2s_node;

	of_node_put(i2s_node);

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
	}

	return ret;
}

static int snd_generic_i2s_mic_remove(struct platform_device *pdev)
{
	pr_alert("remove i2s mic\n");
	return snd_soc_unregister_card(&snd_generic_i2s_mic);
}

static const struct of_device_id snd_generic_i2s_mic_of_match[] = {
        { .compatible = "rpi,generic-i2s-mic", },
        {},
};
MODULE_DEVICE_TABLE(of, snd_generic_i2s_mic_of_match);

static struct platform_driver snd_generic_i2s_mic_driver = {
	.driver = {
		.name   = "snd-i2s-mic",
		.owner  = THIS_MODULE,
		.of_match_table = snd_generic_i2s_mic_of_match,
	},
	.probe	        = snd_generic_i2s_mic_probe,
	.remove	        = snd_generic_i2s_mic_remove,
};

module_platform_driver(snd_generic_i2s_mic_driver);

MODULE_AUTHOR("Christoph Orth");
MODULE_DESCRIPTION("ASoC Driver for Raspberry Pi connected to a generic i2s mic");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:generic-i2s-mic");
