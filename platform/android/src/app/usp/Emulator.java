package app.usp;

import java.nio.ByteBuffer;

public class Emulator
{
	static
	{
		System.loadLibrary("usp");
	}
	synchronized native void	Init();
	synchronized native void	Done();
	synchronized native void	UpdateVideo(ByteBuffer buf);
	synchronized native int		UpdateAudio(ByteBuffer buf);
	synchronized native void	OnKey(char key, boolean down);

	public static Emulator the = new Emulator();
}
