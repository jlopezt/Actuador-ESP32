move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move SecuenciadorConfig.json SecuenciadorConfig.json.sav
move EntradasSalidasConfig.json EntradasSalidas.json.sav

copy Config.json.desarrollo Config.json
copy WiFiConfig.json.desarrollo WiFiConfig.json
copy MQTTConfig.json.desarrollo MQTTConfig.json
copy SecuenciadorConfig.json.desarrollo SecuenciadorConfig.json
copy EntradasSalidasConfig.json.desarrollo EntradasSalidasConfig.json

del Config.json.sav
del MQTTConfig.json.sav
del WiFiConfig.json.sav
del SecuenciadorConfig.json.sav
del EntradasSalidas.json.sav

date /T>AA_DESARROLLO
time /T>>AA_DESARROLLO 
del AA_PRODUCCION