move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move SecuenciadorConfig.json SecuenciadorConfig.json.sav
move SalidasConfig.json SalidasConfig.json.sav
move EntradasConfig.json EntradasConfig.json.sav

copy Config.json.desarrollo Config.json
copy WiFiConfig.json.desarrollo WiFiConfig.json
copy MQTTConfig.json.desarrollo MQTTConfig.json
copy SecuenciadorConfig.json.desarrollo SecuenciadorConfig.json
copy SalidasConfig.json.desarrollo SalidasConfig.json
copy EntradasConfig.json.desarrollo EntradasConfig.json

del Config.json.sav
del MQTTConfig.json.sav
del WiFiConfig.json.sav
del SecuenciadorConfig.json.sav
del SalidasConfig.json.sav
del EntradasConfig.json.sav

date /T>AA_DESARROLLO
time /T>>AA_DESARROLLO 
del AA_PRODUCCION
