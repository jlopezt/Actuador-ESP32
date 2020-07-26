move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move SecuenciadorConfig.json SecuenciadorConfig.json.sav
move SalidasConfig.json SalidasConfig.json.sav
move EntradasConfig.json EntradasConfig.json.sav

copy Config.json.produccion Config.json
copy WiFiConfig.json.produccion WiFiConfig.json
copy MQTTConfig.json.produccion MQTTConfig.json
copy SecuenciadorConfig.json.produccion SecuenciadorConfig.json
copy SalidasConfig.json.produccion SalidasConfig.json
copy EntradasConfig.json.produccion EntradasConfig.json

del Config.json.sav
del MQTTConfig.json.sav
del WiFiConfig.json.sav
del SecuenciadorConfig.json.sav
del SalidasConfig.json.sav
del EntradasConfig.json.sav
 
date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
del AA_DESARROLLO 
