package app.usp;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

public class Preferences extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
	final static private String select_joystick_id = "joystick";
	final static private String sound_source_id = "sound";
	final static private String sound_chip_id = "sound chip";
	final static private String sound_chip_stereo_id = "ay stereo";
	final static private String select_drive_id = "drive";
	final static private String tape_id = "tape";
	final static private String tape_fast_id = "fast tape";
	final static private String mode_48k_id = "mode 48k";
	final static public String select_zoom_id = "zoom";
	final static public String filtering_id = "filtering";
	final static public String select_skip_frames_id = "skip frames";
	final static public String use_sensor_id = "use sensor";
	final static public String use_keyboard_id = "use keyboard";
	private ListPreference select_joystick;
	private ListPreference sound_source;
	private ListPreference sound_chip;
	private ListPreference sound_chip_stereo;
	private ListPreference select_drive;
	private Preference tape;
	private CheckBoxPreference tape_fast;
	private CheckBoxPreference mode_48k;
	private ListPreference select_zoom;
	private CheckBoxPreference filtering;
	private ListPreference select_skip_frames;
	private CheckBoxPreference use_sensor;
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
        select_joystick = (ListPreference)getPreferenceScreen().findPreference(select_joystick_id);
        sound_source = (ListPreference)getPreferenceScreen().findPreference(sound_source_id);
        sound_chip = (ListPreference)getPreferenceScreen().findPreference(sound_chip_id);
        sound_chip_stereo = (ListPreference)getPreferenceScreen().findPreference(sound_chip_stereo_id);
        select_drive = (ListPreference)getPreferenceScreen().findPreference(select_drive_id);
        tape = (Preference)getPreferenceScreen().findPreference(tape_id);
        tape_fast = (CheckBoxPreference)getPreferenceScreen().findPreference(tape_fast_id);
        mode_48k = (CheckBoxPreference)getPreferenceScreen().findPreference(mode_48k_id);
        select_zoom = (ListPreference)getPreferenceScreen().findPreference(select_zoom_id);
        filtering = (CheckBoxPreference)getPreferenceScreen().findPreference(filtering_id);
        select_skip_frames = (ListPreference)getPreferenceScreen().findPreference(select_skip_frames_id);
        use_sensor = (CheckBoxPreference)getPreferenceScreen().findPreference(use_sensor_id);
	}
	private void UpdateDescs()
	{
		select_joystick.setSummary(select_joystick.getEntry());
		sound_source.setSummary(sound_source.getEntry());
		boolean is_AY = Emulator.the.GetOptionInt(sound_source_id) == 1; // AY
		sound_chip.setSummary(sound_chip.getEntry());				sound_chip.setEnabled(is_AY);
		sound_chip_stereo.setSummary(sound_chip_stereo.getEntry());	sound_chip_stereo.setEnabled(is_AY);
		select_drive.setSummary(select_drive.getEntry());
		switch(Emulator.the.TapeState())
		{
		case 0:	tape.setSummary(R.string.tape_na);		tape.setEnabled(false);	break;
		case 1:	tape.setSummary(R.string.tape_stopped);	tape.setEnabled(true);	break;
		case 2:	tape.setSummary(R.string.tape_started);	tape.setEnabled(true);	break;
		}
		select_zoom.setSummary(select_zoom.getEntry());
        select_skip_frames.setSummary(select_skip_frames.getEntry());
	}
	@Override
	protected void onResume()
	{
		super.onResume();
		select_joystick.setValueIndex(Emulator.the.GetOptionInt(select_joystick_id));
		sound_source.setValueIndex(Emulator.the.GetOptionInt(sound_source_id));
		sound_chip.setValueIndex(Emulator.the.GetOptionInt(sound_chip_id));
		sound_chip_stereo.setValueIndex(Emulator.the.GetOptionInt(sound_chip_stereo_id));
		select_drive.setValueIndex(Emulator.the.GetOptionInt(select_drive_id));
		tape_fast.setChecked(Emulator.the.GetOptionBool(tape_fast_id));
		mode_48k.setChecked(Emulator.the.GetOptionBool(mode_48k_id));
		select_zoom.setValueIndex(Emulator.the.GetOptionInt(select_zoom_id));
		filtering.setChecked(Emulator.the.GetOptionBool(filtering_id));
		int v = Emulator.the.GetOptionInt(select_skip_frames_id);
		select_skip_frames.setValueIndex(v);
		use_sensor.setChecked(Emulator.the.GetOptionBool(use_sensor_id));
		getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
		UpdateDescs();
	}
	@Override
	protected void onPause()
	{
		getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
		Emulator.the.StoreOptions();
		super.onPause();
	}
	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key)
	{
		if(key.equals(select_joystick_id))
		{
			Emulator.the.SetOptionInt(select_joystick_id, Integer.parseInt(select_joystick.getValue()));
			UpdateDescs();
		}
		else if(key.equals(sound_source_id))
		{
			Emulator.the.SetOptionInt(sound_source_id, Integer.parseInt(sound_source.getValue()));
			UpdateDescs();
		}
		else if(key.equals(sound_chip_id))
		{
			Emulator.the.SetOptionInt(sound_chip_id, Integer.parseInt(sound_chip.getValue()));
			UpdateDescs();
		}
		else if(key.equals(sound_chip_stereo_id))
		{
			Emulator.the.SetOptionInt(sound_chip_stereo_id, Integer.parseInt(sound_chip_stereo.getValue()));
			UpdateDescs();
		}
		else if(key.equals(select_drive_id))
		{
			Emulator.the.SetOptionInt(select_drive_id, Integer.parseInt(select_drive.getValue()));
			UpdateDescs();
		}
		else if(key.equals(mode_48k_id))
		{
			Emulator.the.SetOptionBool(mode_48k_id, mode_48k.isChecked());
			UpdateDescs();
		}
		else if(key.equals(tape_fast_id))
		{
			Emulator.the.SetOptionBool(tape_fast_id, tape_fast.isChecked());
			UpdateDescs();
		}
		else if(key.equals(select_zoom_id))
		{
			Emulator.the.SetOptionInt(select_zoom_id, Integer.parseInt(select_zoom.getValue()));
			UpdateDescs();
		}
		else if(key.equals(filtering_id))
		{
			Emulator.the.SetOptionBool(filtering_id, filtering.isChecked());
			UpdateDescs();
		}
		else if(key.equals(select_skip_frames_id))
		{
			Emulator.the.SetOptionInt(select_skip_frames_id, Integer.parseInt(select_skip_frames.getValue()));
			UpdateDescs();
		}
		else if(key.equals(use_sensor_id))
		{
			Emulator.the.SetOptionBool(use_sensor_id, use_sensor.isChecked());
			UpdateDescs();
		}
	}
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference)
	{
		if(preference.getKey().equals(tape_id))
		{
			Emulator.the.TapeToggle();
			UpdateDescs();
		}
		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}
