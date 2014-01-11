var devices = document.getElementById("devices");
var details = document.getElementById("details");

dList =	[{"name":"Foo","id":1},
		{"name":"Bar","id":2},
		{"name":"Baz","id":3},
		{"name":"Qux","id":4},
		{"name":"Quxom","id":5},
		{"name":"Lorem","id":6},
		{"name":"Ipsum","id":7},
		{"name":"Dolor","id":8},
		{"name":"Sit","id":9},
		{"name":"Amet","id":10}];

function appendDevice(name, id)
{
	placeholder = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque semper.";
	
	link = document.createElement("a");
	theDiv = document.createElement("div");
	header = document.createElement("h3");
	text = document.createElement("p");
	spark = document.createElement("div");
	
	devices.appendChild(link);
	link.appendChild(theDiv);
	theDiv.appendChild(header);
	theDiv.appendChild(spark);
	//theDiv.appendChild(text);
	
	link.setAttribute("href", "javascript:void(0)");
	link.setAttribute("onClick", "detailsFor(\""+name+"\", "+id.toString()+");");
	
	theDiv.setAttribute("class","device");
	
	header.appendChild(document.createTextNode(name));
	
	text.appendChild(document.createTextNode(placeholder));
	
	spark.setAttribute("class", "bgChart");
}

function closeDetails()
{
	devices.style.height = "90%";
	details.style.height = "0";
}

function openDetails(name,id)
{
	devices.style.height = "50%";
	details.style.height = "50%";
	document.getElementById("devName").innerHTML = "Device: "+name;
	document.getElementById("devMAC").innerHTML = "MAC: "+id.toString();
}

function detailsFor(name, id)
{
	closeDetails();
	setTimeout("openDetails(\""+name+"\","+id.toString()+");",250);
}

function powerFor(id)
{
	return [];
}

for (var i=0;i<dList.length;i++)
{
	appendDevice(dList[i].name,dList[i].id);
}

graphs = document.getElementsByClassName("bgChart");

//console.log($(".bgChart"));

function drawCharts()
{
	for (i=0; i<graphs.length; i+=1)
	{
		$(graphs[i]).sparkline([5,6,7,9,9,5,3,2,2,4,6,7], {
		width: '100%',
		height: '100%',
		type: 'line',
		spotColor: undefined,
		minSpotColor: undefined,
		maxSpotColor: undefined,
		highlightSpotColor: undefined,
		highlightLineColor: undefined});
	}
}

drawCharts();
