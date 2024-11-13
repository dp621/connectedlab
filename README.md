# Connected Lab
Monitor, log and analyze your lab equipment, process or metric

Includes software platform and example hardware devices

Software comes preconfigured with a distillation dashboard as seen below:

![image](https://user-images.githubusercontent.com/55506016/160466747-2a379854-ff16-4cc9-a22f-0d43c5425f79.png)


To Run:
- Install docker & docker-compose
- docker-compose up
- Open browser to http://localhost:3000

To simulate a short path distillation system:
- node mqtt2influx/spd_test.js

Hardware MQTT Protocol:
- Topic: 'labData'
- Data Format: {"device":"<name>", "val":<val>, "unit":"<unit>"}
