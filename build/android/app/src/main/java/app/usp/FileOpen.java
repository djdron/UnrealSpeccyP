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

import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.fragment.app.FragmentActivity;
import androidx.viewpager2.widget.ViewPager2;
import com.google.android.material.tabs.TabLayout;
import com.google.android.material.tabs.TabLayoutMediator;

public class FileOpen extends FragmentActivity
{
	private static int active_tab = 0;
	private ViewPager2 viewPager;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.file_open);

		View rootLayout = findViewById(android.R.id.content);
		ViewCompat.setOnApplyWindowInsetsListener(rootLayout, (view, insets) -> {
			Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
			view.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
			return insets;
		});

		TabLayout tabLayout = findViewById(R.id.tabLayout);
		viewPager = findViewById(R.id.viewPager);

		FileTabsAdapter tabsAdapter = new FileTabsAdapter(this);
		viewPager.setAdapter(tabsAdapter);

		new TabLayoutMediator(tabLayout, viewPager, (tab, position) -> {
			switch(position)
			{
				case 0: tab.setText(R.string.file_system); break;
				case 1: tab.setText(R.string.vtrdos); break;
				case 2: tab.setText(R.string.wos); break;
				case 3: tab.setText(R.string.rzx); break;
				case 4: tab.setText(R.string.bbb); break;
			}
		}).attach();
		viewPager.setCurrentItem(active_tab, false);
	}

	@Override
	public void onDestroy()
	{
		if(viewPager != null)
			active_tab = viewPager.getCurrentItem();
		super.onDestroy();
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		if(item.getItemId() == android.R.id.home)
		{
			getOnBackPressedDispatcher().onBackPressed();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
