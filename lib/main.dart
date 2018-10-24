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
  bool _onOff1 = false;
  bool _highPerformance1 = false;
  void _onChange1(bool value) => setState(() => _onOff1 = value);
  void _powerChange1(bool value) => setState(() => _highPerformance1 = value);


  Widget build(BuildContext context) {
    double _powerUse = 0.4;
    return Scaffold(
        appBar: AppBar(
          title: Text('Appliance Power Monitor'),
        ),
        body: Center(
          child: 
          OutletColumn(
            powerUse: _powerUse,
            onOff: _onOff1,
            highPerformance: _highPerformance1,
            onChanged1: _onChange1,
            onChanged2: _powerChange1,
          ),
        )
      );
  }
}

class OutletColumn extends StatelessWidget{
    OutletColumn({Key key, this.powerUse, this.onOff, this.highPerformance, this.onChanged1, this.onChanged2}): super(key: key);

    final double powerUse;
    final bool onOff;
    final bool highPerformance;
    final ValueChanged<bool> onChanged1;
    final ValueChanged<bool> onChanged2;

  Widget build(BuildContext context)
  {
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
