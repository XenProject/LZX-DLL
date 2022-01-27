# LZX-DLL
Taken from xenia, compiled into a dll for use with csharp / Xenious

To Use in .net (c#) 4.8:


  [DllImport("LZX.dll", EntryPoint = "Decompress", CallingConvention = CallingConvention.Cdecl)]

  public static extern int Decompress(byte[] CompData, int CDSize, [In][Out] byte[] OutputData, int ODSize, uint WindowSize);
  
  [DllImport("LZX.dll", EntryPoint = "Version", CallingConvention = CallingConvention.Cdecl)]
  
  public static extern UInt32 Version();
