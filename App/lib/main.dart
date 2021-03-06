import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart' as MQTT;
import 'dart:async';



void main() => runApp(MyApp());


class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Appliance Power Monitor',
      theme: new ThemeData(
        scaffoldBackgroundColor: Colors.white,
        primaryColor: Colors.blue,
      ),
      home: PowerMonitor(),
    );
  }
}

class PowerMonitor extends StatefulWidget {
  @override
  PowerMonitorState createState() => new PowerMonitorState();
}

class PowerMonitorState extends State<PowerMonitor> {
  bool mqttConnected = false;
  
  MQTT.MqttClient client;
  StreamSubscription subscription;

  double _powerUse1 = 0.0;
  double _powerUse2 = 0.0;
  double _powerUse3 = 0.0;
  double _powerUse4 = 0.0;

  bool _onOff1 = false;
  bool _highPerformance1 = false;
  void _onChange1(bool value)
  {
    setState(() => _onOff1 = value);
    publishSwitchChange("esp32/outlet0/onoff", value);
  } 
  void _powerChange1(bool value) 
    {
    setState(() => _highPerformance1 = value);
    publishSwitchChange("esp32/outlet0/hpm", value);
  } 

  bool _onOff2 = false;
  bool _highPerformance2 = false;
  void _onChange2(bool value)
  {
    setState(() => _onOff2 = value);
    publishSwitchChange("esp32/outlet1/onoff", value);
  }

  void _powerChange2(bool value) 
  {
    setState(() => _highPerformance2 = value);
    publishSwitchChange("esp32/outlet1/hpm", value);
  }

  bool _onOff3 = false;
  bool _highPerformance3 = false;
  void _onChange3(bool value)
  {
    setState(() => _onOff3 = value);
    publishSwitchChange("esp32/outlet2/onoff", value);
  } 
  void _powerChange3(bool value) 
  {
    setState(() => _highPerformance3 = value);
    publishSwitchChange("esp32/outlet2/hpm", value);
  }

  bool _onOff4 = false;
  bool _highPerformance4 = false;
  void _onChange4(bool value)
  {
    setState(() => _onOff4 = value);
    publishSwitchChange("esp32/outlet3/onoff", value);
  } 
  void _powerChange4(bool value) 
  {
    setState(() => _highPerformance4 = value);
    publishSwitchChange("esp32/outlet3/hpm", value);
  } 
  Future <int> mqttConnect() async {
    client = MQTT.MqttClient.withPort('m15.cloudmqtt.com', 'BuqqApp', 11322);
    client.setProtocolV311();
    client.logging(on: true);

    await client.connect("akyumnii","Z2HnUN3RumXD");
    if (client.connectionStatus.state == MQTT.ConnectionState.connected) {
      print("iotcore client connected");
      _subscribeToTopic("esp32/outlet0/data");
      _subscribeToTopic("esp32/outlet1/data");
      _subscribeToTopic("esp32/outlet2/data");
      _subscribeToTopic("esp32/outlet3/data");
      subscription = client.updates.listen(_onMessage); 
    } else {
    print(
    "ERROR iotcore client connection failed - disconnecting, state is ${client
    .connectionStatus}");
    client.disconnect();
    }
    return 0;
  }

  void publishSwitchChange(String topic, bool value)
  {
    String payload = value ? "true":"false";

    final MQTT.MqttClientPayloadBuilder builder =
        MQTT.MqttClientPayloadBuilder();
    builder.addString(payload);
    client.publishMessage(
      topic,
      MQTT.MqttQos.atLeastOnce,
      builder.payload,
    );
  }

  void _subscribeToTopic(String topic) {
    if (client.connectionStatus.state == MQTT.ConnectionState.connected) {
      setState(() {
          print('Subscribing to ${topic.trim()}');
          client.subscribe(topic, MQTT.MqttQos.exactlyOnce);
      });
    }
  }

void _onMessage(List<MQTT.MqttReceivedMessage> event) {
    print(event.length);
    final MQTT.MqttPublishMessage recMess =
        event[0].payload as MQTT.MqttPublishMessage;
    final String message =
        MQTT.MqttPublishPayload.bytesToStringAsString(recMess.payload.message);

    /// The above may seem a little convoluted for users only interested in the
    /// payload, some users however may be interested in the received publish message,
    /// lets not constrain ourselves yet until the package has been in the wild
    /// for a while.
    /// The payload is a byte buffer, this will be specific to the topic
    print('MQTT message: topic is <${event[0].topic}>, '
        'payload is <-- ${message} -->');
    print(client.connectionStatus.state);
    
      if(event[0].topic == "esp32/outlet0/data")
      {
        setState(() {
        print("power1 updated.");
        _powerUse1 =  double.parse(message);
        });
      }   
      if(event[0].topic == "esp32/outlet1/data")
      {
        setState(() {
        print("power2 updated.");
        _powerUse2 =  double.parse(message);
        });
      }
      if(event[0].topic == "esp32/outlet2/data")
      {
        setState(() {
        print("power3 updated.");
        _powerUse3 =  double.parse(message);
        });
      }
      if(event[0].topic == "esp32/outlet3/data")
      {
        setState(() {
        print("power4 updated.");
        _powerUse4 =  double.parse(message);
        });
      }
}

 Widget build(BuildContext context) {
   if(!mqttConnected)
   {
     mqttConnect();  
     mqttConnected = true;
   }
    
    return DefaultTabController(
      length: 4,
      child: Scaffold(
          appBar: AppBar(
              
              bottom: TabBar(
                tabs: [
                Tab(text: '1'),
                Tab(text: '2'),
                Tab(text: '3'),
                Tab(text: '4'),
              ]
              ),
              title: Text('Appliance Power Monitor'),
              ),
          body: TabBarView(children: [
            new OutletColumn(
              powerUse: _powerUse1,
              onOff: _onOff1,
              highPerformance: _highPerformance1,
              onChanged1: _onChange1,
              onChanged2: _powerChange1,
            ),

            new OutletColumn(
              powerUse: _powerUse2,
              onOff: _onOff2,
              highPerformance: _highPerformance2,
              onChanged1: _onChange2,
              onChanged2: _powerChange2,
            ),

            new OutletColumn(
              powerUse: _powerUse3,
              onOff: _onOff3,
              highPerformance: _highPerformance3,
              onChanged1: _onChange3,
              onChanged2: _powerChange3,
            ),

            new OutletColumn(
              powerUse: _powerUse4,
              onOff: _onOff4,
              highPerformance: _highPerformance4,
              onChanged1: _onChange4,
              onChanged2: _powerChange4,
            ),
          ])),
    );
  }
}

class OutletColumn extends StatelessWidget {
  OutletColumn(
      {Key key,
      this.powerUse,
      this.onOff,
      this.highPerformance,
      this.onChanged1,
      this.onChanged2})
      : super(key: key);

  final double powerUse;
  final bool onOff;
  final bool highPerformance;
  final ValueChanged<bool> onChanged1;
  final ValueChanged<bool> onChanged2;

  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        new Row(
            crossAxisAlignment: CrossAxisAlignment.end,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                powerUse.toString(),
                style: TextStyle(
                  fontSize: 80.0,
                  fontWeight: FontWeight.w400,
                ),
              ),
              Text(
                'W',
                style: TextStyle(
                  fontSize: 28.0,
                  fontWeight: FontWeight.w200,
                ),
              ),
            ]),
        new Container(
          child: Column(children: [
            new SwitchListTile(
              value: onOff,
              onChanged: onChanged1,
              title: new Text('Outlet On/Off',
                  style: new TextStyle(
                      fontWeight: FontWeight.bold, color: Colors.black)),
            ),
            new SwitchListTile(
              value: highPerformance,
              onChanged: onChanged2,
              title: new Text('High Performance Mode',
                  style: new TextStyle(
                      fontWeight: FontWeight.bold, color: Colors.black)),
            ),
          ]),
        ),
      ],
    );
  }
}
 


