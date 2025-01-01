--
-- Table structure for table `guild_auras`
--

DROP TABLE IF EXISTS `guild_auras`;
CREATE TABLE `guild_auras`  (
  `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `spell_aura_id` int(10) UNSIGNED NOT NULL,
  `required_guild_level` int(10) UNSIGNED NOT NULL DEFAULT 1,
  `description` tinytext CHARACTER SET utf8 ,
  PRIMARY KEY (`id`)
) ENGINE = InnoDB AUTO_INCREMENT = 0 CHARACTER SET = utf8;