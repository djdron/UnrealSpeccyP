/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

import android.app.UiModeManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Build;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.Preference;
import androidx.preference.PreferenceGroup;
import androidx.preference.SwitchPreferenceCompat;
import androidx.preference.ListPreference;
import androidx.fragment.app.FragmentActivity;
import androidx.preference.PreferenceFragmentCompat;
import androidx.recyclerview.widget.LinearLayoutManager;

import android.view.MenuItem;
import android.view.View;
import android.view.Window;

public class Preferences extends FragmentActivity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		getWindow().requestFeature(Window.FEATURE_OPTIONS_PANEL);
		super.onCreate(savedInstanceState);
		getSupportFragmentManager().beginTransaction().replace(android.R.id.content, new PreferencesFragment()).commit();
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		if(item.getItemId() == android.R.id.home)
		{
			getOnBackPressedDispatcher().onBackPressed();
		}
		return super.onOptionsItemSelected(item);
	}

	final static public String av_timer_sync_id = "av timer sync";
	final static public String skip_frames_id = "skip frames";
	final static public String use_sensor_id = "use sensor";
	final static public String use_keyboard_id = "use keyboard";

	public static class PreferencesFragment extends PreferenceFragmentCompat implements OnSharedPreferenceChangeListener
	{
		private ListPreference select_joystick;
		private SwitchPreferenceCompat use_sensor;
		private ListPreference sound_chip;
		private ListPreference sound_chip_stereo;
		private ListPreference save_slot;
		private Preference save_file;
		private ListPreference select_drive;
		private Preference tape;
		private SwitchPreferenceCompat auto_play_image;
		private SwitchPreferenceCompat tape_fast;
		private SwitchPreferenceCompat mode_48k;
		private SwitchPreferenceCompat reset_to_service_rom;
		private ListPreference select_zoom;
		private SwitchPreferenceCompat filtering;
		private SwitchPreferenceCompat gigascreen;
		private SwitchPreferenceCompat black_and_white;
		private SwitchPreferenceCompat av_timer_sync;
		private ListPreference skip_frames;
		private ListPreference theme;

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
		final static private String theme_id = "theme";

		private static int scroll_stored = 0;
		private static int scroll_offset_stored = 0;

		@Override
		public void onCreatePreferences(@Nullable Bundle savedInstanceState, @Nullable String rootKey)
		{
			setPreferencesFromResource(R.xml.preferences, rootKey);
			removeIconSpace(getPreferenceScreen());
			select_joystick = findPreference(select_joystick_id);
			use_sensor = findPreference(use_sensor_id);
			sound_chip = findPreference(sound_chip_id);
			sound_chip_stereo = findPreference(sound_chip_stereo_id);
			auto_play_image = findPreference(auto_play_image_id);
			save_slot = findPreference(save_slot_id);
			save_file = findPreference(save_file_id);
			select_drive = findPreference(select_drive_id);
			tape = findPreference(tape_id);
			tape_fast = findPreference(tape_fast_id);
			mode_48k = findPreference(mode_48k_id);
			reset_to_service_rom = findPreference(reset_to_service_rom_id);
			select_zoom = findPreference(select_zoom_id);
			filtering = findPreference(filtering_id);
			gigascreen = findPreference(gigascreen_id);
			black_and_white = findPreference(black_and_white_id);
			av_timer_sync = findPreference(av_timer_sync_id);
			skip_frames = findPreference(skip_frames_id);
			theme = findPreference(theme_id);
			LoadValues();
			UpdateDescs();

			tape.setOnPreferenceClickListener(preference -> {
				Emulator.the.TapeToggle();
				UpdateDescs();
				return true;
			});
			save_file.setOnPreferenceClickListener(preference -> {
				Emulator.the.SaveFile();
				UpdateDescs();
				return true;
			});
		}
		private void removeIconSpace(Preference preference)
		{
			preference.setIconSpaceReserved(false);
			if(preference instanceof PreferenceGroup)
			{
				PreferenceGroup group = (PreferenceGroup)preference;
				for(int i = 0; i < group.getPreferenceCount(); i++)
				{
					removeIconSpace(group.getPreference(i));
				}
			}
		}
		@Override
		public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState)
		{
			super.onViewCreated(view, savedInstanceState);
			if(scroll_stored != 0 || scroll_offset_stored != 0)
			{
				((LinearLayoutManager) getListView().getLayoutManager()).scrollToPositionWithOffset(scroll_stored, scroll_offset_stored);
			}
		}
		@Override
		public void onResume()
		{
			super.onResume();
			getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
		}
		@Override
		public void onPause()
		{
			super.onPause();
			Emulator.the.StoreOptions();
			LinearLayoutManager lm = (LinearLayoutManager)getListView().getLayoutManager();
			scroll_stored = lm.findFirstVisibleItemPosition();
			View topView = lm.getChildAt(0);
			scroll_offset_stored = (topView == null) ? 0 : (topView.getTop() - getListView().getPaddingTop());
			getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
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
			switch(Emulator.the.TapeState())
			{
				case 0:	tape.setSummary(R.string.tape_na);		tape.setEnabled(false);	break;
				case 1:	tape.setSummary(R.string.tape_stopped);	tape.setEnabled(true);	break;
				case 2:	tape.setSummary(R.string.tape_started);	tape.setEnabled(true);	break;
			}
			save_file.setEnabled(Emulator.the.DiskChanged());
		}
		@Override
		public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key)
		{
			switch(key)
			{
			case select_joystick_id:
				Emulator.the.SetOptionInt(select_joystick_id, Integer.parseInt(select_joystick.getValue()));
				break;
			case sound_chip_id:
				Emulator.the.SetOptionInt(sound_chip_id, Integer.parseInt(sound_chip.getValue()));
				break;
			case sound_chip_stereo_id:
				Emulator.the.SetOptionInt(sound_chip_stereo_id, Integer.parseInt(sound_chip_stereo.getValue()));
				break;
			case auto_play_image_id:
				Emulator.the.SetOptionBool(auto_play_image_id, auto_play_image.isChecked());
				break;
			case save_slot_id:
				Emulator.the.SetOptionInt(save_slot_id, Integer.parseInt(save_slot.getValue()));
				break;
			case select_drive_id:
				Emulator.the.SetOptionInt(select_drive_id, Integer.parseInt(select_drive.getValue()));
				break;
			case mode_48k_id:
				Emulator.the.SetOptionBool(mode_48k_id, mode_48k.isChecked());
				break;
			case reset_to_service_rom_id:
				Emulator.the.SetOptionBool(reset_to_service_rom_id, reset_to_service_rom.isChecked());
				break;
			case tape_fast_id:
				Emulator.the.SetOptionBool(tape_fast_id, tape_fast.isChecked());
				break;
			case select_zoom_id:
				Emulator.the.SetOptionInt(select_zoom_id, Integer.parseInt(select_zoom.getValue()));
				break;
			case use_sensor_id:
				Emulator.the.SetOptionBool(use_sensor_id, use_sensor.isChecked());
				break;
			case filtering_id:
				Emulator.the.SetOptionBool(filtering_id, filtering.isChecked());
				break;
			case gigascreen_id:
				Emulator.the.SetOptionBool(gigascreen_id, gigascreen.isChecked());
				break;
			case black_and_white_id:
				Emulator.the.SetOptionBool(black_and_white_id, black_and_white.isChecked());
				break;
			case av_timer_sync_id:
				Emulator.the.SetOptionBool(av_timer_sync_id, av_timer_sync.isChecked());
				break;
			case skip_frames_id:
				Emulator.the.SetOptionInt(skip_frames_id, Integer.parseInt(skip_frames.getValue()));
				break;
			case theme_id:
				UiModeManager uiModeManager = (UiModeManager)getActivity().getSystemService(Context.UI_MODE_SERVICE);
				int mode = theme.getValue().equals("dark") ? UiModeManager.MODE_NIGHT_YES : (theme.getValue().equals("light") ? UiModeManager.MODE_NIGHT_NO : UiModeManager.MODE_NIGHT_AUTO);
				if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) // Android 12+
					uiModeManager.setApplicationNightMode(mode);
				else
					uiModeManager.setNightMode(mode);
				break;
			}
		}
	}
}
