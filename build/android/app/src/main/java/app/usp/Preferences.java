/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2020 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.view.MenuItem;

public class Preferences extends PreferenceActivity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		getActionBar().setDisplayHomeAsUpEnabled(true);
		getFragmentManager().beginTransaction().replace(android.R.id.content, new PreferencesFragment()).commit();
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch(item.getItemId())
		{
			case android.R.id.home:	onBackPressed(); break;
		}
		return super.onOptionsItemSelected(item);
	}

	final static public String av_timer_sync_id = "av timer sync";
	final static public String skip_frames_id = "skip frames";
	final static public String use_sensor_id = "use sensor";
	final static public String use_keyboard_id = "use keyboard";

	public static class PreferencesFragment extends PreferenceFragment implements OnSharedPreferenceChangeListener
	{
		private ListPreference select_joystick;
		private CheckBoxPreference use_sensor;
		private ListPreference sound_chip;
		private ListPreference sound_chip_stereo;
		private ListPreference save_slot;
		private Preference save_file;
		private ListPreference select_drive;
		private Preference tape;
		private CheckBoxPreference auto_play_image;
		private CheckBoxPreference tape_fast;
		private CheckBoxPreference mode_48k;
		private CheckBoxPreference reset_to_service_rom;
		private ListPreference select_zoom;
		private CheckBoxPreference filtering;
		private CheckBoxPreference gigascreen;
		private CheckBoxPreference black_and_white;
		private CheckBoxPreference av_timer_sync;
		private ListPreference skip_frames;

		final static private String select_joystick_id = "joystick";
		final static private String sound_chip_id = "sound chip";
		final static private String sound_chip_stereo_id = "ay stereo";
		final static private String auto_play_image_id = "auto play image";
		final static private String save_slot_id = "save slot";
		final static private String save_file_id = "save file";
		final static private String select_drive_id = "drive";
		final static private String tape_id = "tape";
		final static private String tape_fast_id = "fast tape";
		final static private String mode_48k_id = "mode 48k";
		final static private String reset_to_service_rom_id = "reset to service rom";
		final static private String select_zoom_id = "zoom";
		final static private String filtering_id = "filtering";
		final static private String gigascreen_id = "gigascreen";
		final static private String black_and_white_id = "black and white";

		@Override
		public void onCreate(Bundle savedInstanceState)
		{
			super.onCreate(savedInstanceState);
			addPreferencesFromResource(R.xml.preferences);
			select_joystick = (ListPreference)getPreferenceManager().findPreference(select_joystick_id);
			use_sensor = (CheckBoxPreference)getPreferenceManager().findPreference(use_sensor_id);
			sound_chip = (ListPreference)getPreferenceManager().findPreference(sound_chip_id);
			sound_chip_stereo = (ListPreference)getPreferenceManager().findPreference(sound_chip_stereo_id);
			auto_play_image = (CheckBoxPreference)getPreferenceManager().findPreference(auto_play_image_id);
			save_slot = (ListPreference)getPreferenceManager().findPreference(save_slot_id);
			save_file = getPreferenceManager().findPreference(save_file_id);
			select_drive = (ListPreference)getPreferenceManager().findPreference(select_drive_id);
			tape = getPreferenceManager().findPreference(tape_id);
			tape_fast = (CheckBoxPreference)getPreferenceManager().findPreference(tape_fast_id);
			mode_48k = (CheckBoxPreference)getPreferenceManager().findPreference(mode_48k_id);
			reset_to_service_rom = (CheckBoxPreference)getPreferenceManager().findPreference(reset_to_service_rom_id);
			select_zoom = (ListPreference)getPreferenceManager().findPreference(select_zoom_id);
			filtering = (CheckBoxPreference)getPreferenceManager().findPreference(filtering_id);
			gigascreen = (CheckBoxPreference)getPreferenceManager().findPreference(gigascreen_id);
			black_and_white = (CheckBoxPreference)getPreferenceManager().findPreference(black_and_white_id);
			av_timer_sync = (CheckBoxPreference)getPreferenceManager().findPreference(av_timer_sync_id);
			skip_frames = (ListPreference)getPreferenceManager().findPreference(skip_frames_id);
			LoadValues();
			UpdateDescs();

			tape.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
				@Override
				public boolean onPreferenceClick(Preference preference) {
					Emulator.the.TapeToggle();
					UpdateDescs();
					return true;
				}
			});
			save_file.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
				@Override
				public boolean onPreferenceClick(Preference preference) {
					Emulator.the.SaveFile();
					UpdateDescs();
					return true;
				}
			});
			getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
		}
		@Override
		public void onDestroy()
		{
			getPreferenceManager().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
			Emulator.the.StoreOptions();
			super.onDestroy();
		}
		private void LoadValues()
		{
			select_joystick.setValueIndex(Emulator.the.GetOptionInt(select_joystick_id));
			use_sensor.setChecked(Emulator.the.GetOptionBool(use_sensor_id));
			sound_chip.setValueIndex(Emulator.the.GetOptionInt(sound_chip_id));
			sound_chip_stereo.setValueIndex(Emulator.the.GetOptionInt(sound_chip_stereo_id));
			auto_play_image.setChecked(Emulator.the.GetOptionBool(auto_play_image_id));
			save_slot.setValueIndex(Emulator.the.GetOptionInt(save_slot_id));
			select_drive.setValueIndex(Emulator.the.GetOptionInt(select_drive_id));
			tape_fast.setChecked(Emulator.the.GetOptionBool(tape_fast_id));
			mode_48k.setChecked(Emulator.the.GetOptionBool(mode_48k_id));
			reset_to_service_rom.setChecked(Emulator.the.GetOptionBool(reset_to_service_rom_id));
			select_zoom.setValueIndex(Emulator.the.GetOptionInt(select_zoom_id));
			filtering.setChecked(Emulator.the.GetOptionBool(filtering_id));
			gigascreen.setChecked(Emulator.the.GetOptionBool(gigascreen_id));
			black_and_white.setChecked(Emulator.the.GetOptionBool(black_and_white_id));
			av_timer_sync.setChecked(Emulator.the.GetOptionBool(av_timer_sync_id));
			skip_frames.setValueIndex(Emulator.the.GetOptionInt(skip_frames_id));
		}
		private void UpdateDescs()
		{
			select_joystick.setSummary(select_joystick.getEntry());
			sound_chip.setSummary(sound_chip.getEntry());
			sound_chip_stereo.setSummary(sound_chip_stereo.getEntry());
			save_slot.setSummary(save_slot.getEntry());
			select_drive.setSummary(select_drive.getEntry());
			switch(Emulator.the.TapeState())
			{
				case 0:	tape.setSummary(R.string.tape_na);		tape.setEnabled(false);	break;
				case 1:	tape.setSummary(R.string.tape_stopped);	tape.setEnabled(true);	break;
				case 2:	tape.setSummary(R.string.tape_started);	tape.setEnabled(true);	break;
			}
			save_file.setEnabled(Emulator.the.DiskChanged());
			select_zoom.setSummary(select_zoom.getEntry());
			skip_frames.setSummary(skip_frames.getEntry());
		}
		@Override
		public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key)
		{
			if(key.equals(select_joystick_id))
			{
				Emulator.the.SetOptionInt(select_joystick_id, Integer.parseInt(select_joystick.getValue()));
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
			else if(key.equals(auto_play_image_id))
				Emulator.the.SetOptionBool(auto_play_image_id, auto_play_image.isChecked());
			else if(key.equals(save_slot_id))
			{
				Emulator.the.SetOptionInt(save_slot_id, Integer.parseInt(save_slot.getValue()));
				UpdateDescs();
			}
			else if(key.equals(select_drive_id))
			{
				Emulator.the.SetOptionInt(select_drive_id, Integer.parseInt(select_drive.getValue()));
				UpdateDescs();
			}
			else if(key.equals(mode_48k_id))
				Emulator.the.SetOptionBool(mode_48k_id, mode_48k.isChecked());
			else if(key.equals(reset_to_service_rom_id))
				Emulator.the.SetOptionBool(reset_to_service_rom_id, reset_to_service_rom.isChecked());
			else if(key.equals(tape_fast_id))
				Emulator.the.SetOptionBool(tape_fast_id, tape_fast.isChecked());
			else if(key.equals(select_zoom_id))
			{
				Emulator.the.SetOptionInt(select_zoom_id, Integer.parseInt(select_zoom.getValue()));
				UpdateDescs();
			}
			else if(key.equals(use_sensor_id))
				Emulator.the.SetOptionBool(use_sensor_id, use_sensor.isChecked());
			else if(key.equals(filtering_id))
				Emulator.the.SetOptionBool(filtering_id, filtering.isChecked());
			else if(key.equals(gigascreen_id))
				Emulator.the.SetOptionBool(gigascreen_id, gigascreen.isChecked());
			else if(key.equals(black_and_white_id))
				Emulator.the.SetOptionBool(black_and_white_id, black_and_white.isChecked());
			else if(key.equals(av_timer_sync_id))
				Emulator.the.SetOptionBool(av_timer_sync_id, av_timer_sync.isChecked());
			else if(key.equals(skip_frames_id))
			{
				Emulator.the.SetOptionInt(skip_frames_id, Integer.parseInt(skip_frames.getValue()));
				UpdateDescs();
			}
		}
	}
}
