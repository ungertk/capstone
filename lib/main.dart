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
  void _onChanged1(bool value) => setState(() => _onOff1 = value);
  void _onChanged2(bool value) => setState(() => _highPerformance1 = value);


  Widget build(BuildContext context) {
    double _powerUse = 0.0;
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
            onChanged1: _onChanged1,
            onChanged2: _onChanged2,
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
    final ValueChanged onChanged1;
    final ValueChanged onChanged2;

    void _handleChange1()
    {
      onChanged1(onOff);
    }
    _handleChange2()
    {
      onChanged2(highPerformance);
    }

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
                  onChanged: _handleChange1,
                  title: new Text('Outlet On/Off',
                      style: new TextStyle(
                          fontWeight: FontWeight.bold, color: Colors.black)),
                ),
                new SwitchListTile(
                  value: highPerformance,
                  onChanged: _handleChange2,
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
