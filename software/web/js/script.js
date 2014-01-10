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
	link.setAttribute("onClick", "detailsFor("+id.toString()+");");
	
	theDiv.setAttribute("class","device");
	
	header.appendChild(document.createTextNode(name));
	
	text.appendChild(document.createTextNode(placeholder));
}

function detailsFor(id)
{
	details.style.height = "50%";
	console.log(id);
	return;
}

for (var i=0;i<100;i++)
{
	appendDevice(i.toString(),i);
}
