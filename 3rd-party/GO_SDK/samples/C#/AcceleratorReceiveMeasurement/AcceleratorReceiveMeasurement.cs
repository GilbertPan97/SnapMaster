/*
 * AcceleratorReceiveMeasurement.cs
 *
 * Gocator 2000 Sample
 * Copyright (C) 2011-2024 by LMI Technologies Inc.
 *
 * Licensed under The MIT License.
 * Redistributions of files must retain the above copyright notice.
 *
 * Purpose: Demonstrates the simple use of the Accelerator by connecting to a sensor and receiving a measurement.
 * This allows processing to be performed on the PC rather than on the sensor.
 */

// Uncomment to enable functionality to assign to the accelerated sensor an IP address of the
// an interface on the accelerator host.
//#define SET_ACCELERATED_SENSOR_ADDR

using System;

#if SET_ACCELERATED_SENSOR_ADDR
using System.Threading;   // For the Thread.Sleep() call.
#endif

using Lmi3d.GoSdk;
using Lmi3d.Zen;
using Lmi3d.Zen.Io;
using Lmi3d.GoSdk.Messages;

static class Constants
{
    public const string SENSOR_IP = "192.168.1.10"; // IP of the physical sensor used for sensor connection GoSystem_FindSensorByIpAddress() call.
    public const uint WEB_PORT = 8081;  // Using non-default web port number
    public const uint RECEIVE_DATA_TIMEOUT_USEC = 30000000; // 30 sec

#if SET_ACCELERATED_SENSOR_ADDR
    public const string ACCELERATOR_IP = "192.168.1.1";  // IP address of one of the accelerating computer's network interfaces.

    // Total poll time is SDK_SENSOR_ADDRESS_UPDATE_POLL_COUNT * SDK_SENSOR_ADDRESS_UPDATE_POLL_PERIOD_MSEC
    public const int SDK_SENSOR_ADDRESS_UPDATE_POLL_PERIOD_MSEC = 20;  // 20 msec
    public const uint SDK_SENSOR_ADDRESS_UPDATE_POLL_COUNT = 150;
#endif
}

namespace AcceleratorReceiveMeasurement
{
    class AcceleratorReceiveMeasurement
    {
        static int Main(string[] args)
        {
            try
            {
                KApiLib.Construct();
                GoSdkLib.Construct();
                GoSystem system = new GoSystem();
                GoAccelerator accelerator = new GoAccelerator();
                GoSensor sensor;
                KIpAddress ipAddress = KIpAddress.Parse(Constants.SENSOR_IP);
                GoDataSet dataSet = new GoDataSet();

#if SET_ACCELERATED_SENSOR_ADDR
                // Need to set the IP of GoAccelerator before starting and attaching it.
                KIpAddress accIpAddress = KIpAddress.Parse(Constants.ACCELERATOR_IP);
                accelerator.Address = accIpAddress;
#endif

                accelerator.WebPort = Constants.WEB_PORT;
                accelerator.Start();
                sensor = system.FindSensorByIpAddress(ipAddress);
                accelerator.Attach(sensor);

#if SET_ACCELERATED_SENSOR_ADDR
                // Wait for SDK sensor object to be updated with new discovery information
                // from the accelerated sensor code's discovery server, which will
                // update the SDK sensor object's address with the accelerated sensor code's
                // IP address that was set above to "accIpAddress".
                {
                    uint checkCount = Constants.SDK_SENSOR_ADDRESS_UPDATE_POLL_COUNT;

                    while (checkCount > 0)
                    {
                        Thread.Sleep(Constants.SDK_SENSOR_ADDRESS_UPDATE_POLL_PERIOD_MSEC);

                        GoAddressInfo sensorAddrInfo = sensor.Address();

                        // Check if it has been updated with the configured accelerated sensor IP address.
                        if (sensorAddrInfo.Address.Equals(accIpAddress))
                        {
                            // Address updated, so connection should work now. Exit loop.
                            break;
                        }
                        checkCount--;
                    }

                    // See how long it took for the address to be updated.
                    //  Time = (SDK_SENSOR_ADDRESS_UPDATE_POLL_COUNT - current checkCount value) * SDK_SENSOR_ADDRESS_UPDATE_POLL_PERIOD_MSEC.
                    Console.WriteLine("Checkount: {0}", checkCount);
                }
#endif

                sensor.Connect();
                system.EnableData(true);
                system.Start();
                // refer to SetupMeasurement.cs for setting up of the measurement tools
                dataSet = system.ReceiveData(Constants.RECEIVE_DATA_TIMEOUT_USEC);
                for (uint i = 0; i < dataSet.Count; i++)
                {

                    GoDataMsg dataObj = (GoDataMsg)dataSet.Get(i);
                    switch (dataObj.MessageType)
                    {
                        case GoDataMessageType.Stamp:
                            {
                                GoStampMsg stampMsg = (GoStampMsg)dataObj;
                                for (uint j = 0; j < stampMsg.Count; j++)
                                {
                                    GoStamp stamp = stampMsg.Get(j);
                                    Console.WriteLine("Frame Index = {0}", stamp.FrameIndex);
                                    Console.WriteLine("Time Stamp = {0}", stamp.Timestamp);
                                    Console.WriteLine("Encoder Value = {0}", stamp.Encoder);
                                }
                            }
                            break;
                        case GoDataMessageType.Measurement:
                            {
                                GoMeasurementMsg measurementMsg = (GoMeasurementMsg)dataObj;
                                for (uint k = 0; k < measurementMsg.Count; ++k)
                                {
                                    GoMeasurementData measurementData = measurementMsg.Get(k);
                                    Console.WriteLine("ID: {0}", measurementMsg.Id);
                                    Console.WriteLine("Value: {0}", measurementData.Value);
                                    Console.WriteLine("Decision: {0}", measurementData.Decision);
                                }
                            }
                            break;
                    }
                }
                system.Stop();
                accelerator.Detach(sensor);
                accelerator.Stop();
            }
            catch (KException ex)
            {
                Console.WriteLine("Error: {0}", ex.ToString());
            }

            // wait for ENTER key
            Console.WriteLine("\nPress ENTER to continue");
            while (Console.ReadKey().Key != ConsoleKey.Enter) { }
            return 1;
        }
    }
}
