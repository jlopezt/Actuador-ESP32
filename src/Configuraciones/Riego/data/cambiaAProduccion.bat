move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move SecuenciadorConfig.json SecuenciadorConfig.json.sav
move EntradasSalidasConfig.json EntradasSalidas.json.sav

copy Config.json.produccion Config.json
copy WiFiConfig.json.produccion WiFiConfig.json
copy MQTTConfig.json.produccion MQTTConfig.json
copy SecuenciadorConfig.json.produccion SecuenciadorConfig.json
copy EntradasSalidasConfig.json.produccion EntradasSalidasConfig.json

del Config.json.sav
del MQTTConfig.json.sav
del WiFiConfig.json.sav
del SecuenciadorConfig.json.sav
del EntradasSalidas.json.sav
 
date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
del AA_DESARROLLO 
