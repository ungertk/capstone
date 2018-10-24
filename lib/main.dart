import 'package:flutter/material.dart';

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
  bool _switchState1 = false;
  bool _switchState2 = false;

  void _onChanged1(bool value) => setState(() => _switchState1 = value);
  void _onChanged2(bool value) => setState(() => _switchState2 = value);

  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text('Appliance Power Monitor'),
        ),
        body: Column(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          //padding: EdgeInsets.all(12.0),
          children: [
            new Row(
                crossAxisAlignment: CrossAxisAlignment.end,
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text(
                    '0.0',
                    style: TextStyle(
                      fontSize: 80.0,
                      fontWeight: FontWeight.w400,
                    ),
                  ),
                  Text(
                    'kWh',
                    style: TextStyle(
                      fontSize: 28.0,
                      fontWeight: FontWeight.w200,
                    ),
                  ),
                ]),
            new Container(
              child: Column(children: [
                new SwitchListTile(
                  value: _switchState1,
                  onChanged: _onChanged1,
                  title: new Text('Outlet On/Off',
                      style: new TextStyle(
                          fontWeight: FontWeight.bold, color: Colors.black)),
                ),
                new SwitchListTile(
                  value: _switchState2,
                  onChanged: _onChanged2,
                  title: new Text('High Performance Mode',
                      style: new TextStyle(
                          fontWeight: FontWeight.bold, color: Colors.black)),
                ),
              ]),
            ),
          ],
        ));
  }
}
