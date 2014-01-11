var devices = document.getElementById("devices");
var details = document.getElementById("details");

console.log(devices.innerHTML);

function appendDevice(name, id)
{
	placeholder = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque semper.";
	
	link = document.createElement("a");
	theDiv = document.createElement("div");
	header = document.createElement("h3");
	text = document.createElement("p");
	
	devices.appendChild(link);
	link.appendChild(theDiv);
	theDiv.appendChild(header);
	theDiv.appendChild(text);
	
	link.setAttribute("href", "javascript:void(0)");
	link.setAttribute("onClick", "detailsFor("+id.toString()+", "+id.toString()+");");
	
	theDiv.setAttribute("class","device");
	
	header.appendChild(document.createTextNode(name));
	
	text.appendChild(document.createTextNode(placeholder));
}

function closeDetails()
{
	devices.style.height = "90%";
	details.style.height = "0";
}

function openDetails()
{
	devices.style.height = "50%";
	details.style.height = "50%";
}

function detailsFor(name, id)
{
	closeDetails();
	setTimeout("openDetails();",250);
	return;
}

for (var i=0;i<100;i++)
{
	appendDevice(i.toString(),i);
}
