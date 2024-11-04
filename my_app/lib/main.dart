import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ESP8266 Sernsor Data',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: SensorDataScreen(),
    );
  }
}

class SensorDataScreen extends StatefulWidget {
  @override
  _SensorDataScreenState createState() => _SensorDataScreenState();
}

class _SensorDataScreenState extends State<SensorDataScreen> {
  double? temperature;
  double? humidity;
  String? errorMessage; // Track errors

  final String baseUrl = 'http://192.168.1.139/data'; // Updated endpoint

  Future<void> fetchSensorData() async {
    try {
      final response = await http.get(Uri.parse('$baseUrl/data'));

      if (response.statusCode == 200) {
        final jsonData = json.decode(response.body);
        setState(() {
          temperature = jsonData['temperature'];
          humidity = jsonData['humidity'];
        });
      } else {
        throw Exception('Failed to load data');
      }
    } catch (e) {
      print('Error: $e');
      setState(() {
        temperature = null;
        humidity = null;
      });
    }
  }

  @override
  void initState() {
    super.initState();
    fetchSensorData(); // Fetch data on load
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('ESP8266 Sensor Data'),
      ),
      body: Center(
        child: temperature != null && humidity != null
            ? Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text(
                    'Temperature: ${temperature!.toStringAsFixed(1)} °C',
                    style: TextStyle(fontSize: 24),
                  ),
                  Text(
                    'Humidity: ${humidity!.toStringAsFixed(1)} %',
                    style: TextStyle(fontSize: 24),
                  ),
                  ElevatedButton(
                    onPressed: fetchSensorData,
                    child: Text('Refresh Data'),
                  ),
                ],
              )
            : errorMessage != null
                ? Text(
                    errorMessage!,
                    style: TextStyle(fontSize: 20, color: Colors.red),
                  )
                : Text(
                    'Fetching data...',
                    style: TextStyle(fontSize: 24),
                  ),
      ),
    );
  }
}
