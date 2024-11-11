# Using Gocator SDK

### Environment Variable Setup
All GoSDK example projects require the environment variable **GO_SDK_4** to be set. This variable should point to the *GO_SDK* directory, for example, *C:\14400-4.0.9.156_SOFTWARE_GO_SDK\GO_SDK*. To set up the environment variable in Windows follow these steps:

1. Press `Win + S` to open the Windows search bar, type **Edit the system environment variables**, and select it from the results.
2. In the System Properties window, click on the **Environment Variables** button at the bottom right.
3. In the Environment Variables window, go to the **User variables** section and click on **New**.
4. In the New User Variable dialog, set the **Variable name** to `GO_SDK_4`
5. Click **Browse Directory** and navigate to the `x.x.x.x_SOFTWARE_GO_SDK\GO_SDK` directory. Select this directory to set it as the **Variable value**.
6. Click **OK** to save all changes.
# Documentation

### API Reference
The complete Gocator SDK API reference can be found in the following file: `x.x.x.x_SOFTWARE_GO_SDK\GO_SDK\doc\GoSdk\Gocator\GoSdk.html`.

### Accessing the User Manual
The user manual can be downloaded or opened as HTML from within the Web interface.

1. In Gocator GUI, navigate to the **Manage** page and click on the **Support** category.
2. Choose either **Open HTML** or **Download PDF** to access the user manual.
3. Once accessed, navigate to the **Development Kits** section and click on the **SDK** subsection to access the GoSDK user manual.

# Examples
Examples demonstrating various operations are provided, each focusing on specific functionalities. For Visual Studio, these examples are available in solution files tailored to different versions of Visual Studio. For instance, `GoSdk-2017.sln` is designed for use with Visual Studio 2017. Additionally, a make file is provided for Linux systems.

### Compiling Examples in Visual Studio

To compile the examples in Visual Studio, you may need to retarget the solution to match the installed Windows SDK version. You can do this by selecting the **Retarget solution** option from the solution context menu.

### Running GoSDK Examples

When running the GoSDK examples, ensure that the required DLLs are copied next to the executable. Typically, only **GoSDK.dll** and **kApi.dll** are necessary. However, for .NET and accelerator features, additional DLLs may be required. Refer to the SDK samples for a list of required DLLs.
