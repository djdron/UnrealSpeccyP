module = null;
statusText = 'unknown';

function moduleDidLoad()
{
	module = document.getElementById('unreal_speccy_portable');
	updateStatus('module_ok');
}

// Handle a message coming from the NaCl module.
function handleMessage(m)
{
	updateStatus(m.data);
	if(m.data == 'resources_ok')
		module.postMessage('run');
	else if(m.data == 'ready')
	{
		var joy = localStorage["joystick"];
		if(joy)
		{
			module.postMessage('joystick:' + joy);
		}
	}
}

function pageDidLoad()
{
	if(module == null)
		updateStatus('loading...');
	else
		updateStatus();
	var joy = localStorage["joystick"];
	if(joy)
	{
		var joystick = document.getElementById("joystick");
		for(var i = 0; i < joystick.children.length; i++)
		{
			var j = joystick.children[i];
			if(j.value == joy)
			{
				j.selected = "true";
				break;
			}
		}
	}
}

function onOpen()
{
    var image_name = document.getElementById("image_name").value;
    if(image_name.length)
    {
		updateStatus('opening...');
	    module.postMessage('open:' + image_name);
	}
}
function onReset()
{
	module.postMessage('reset');
}
function onJoystick()
{
    var joystick = document.getElementById("joystick");
    var joy = joystick.options[joystick.selectedIndex].value;
	module.postMessage('joystick:' + joy);
	localStorage["joystick"] = joy;
}

function updateStatus(opt_message)
{
	if(opt_message)
		statusText = opt_message;
	var status = document.getElementById('status');
	if(status)
	{
		status.innerHTML = statusText;
	}
}
