package app.usp;

import com.google.android.material.color.DynamicColors;

public class Application extends android.app.Application
{
	@Override
	public void onCreate()
	{
		super.onCreate();
		DynamicColors.applyToActivitiesIfAvailable(this);
	}
}
