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
	var start = id;
	var arr = [];
	for (var x = 0; x<200; x++)
	{
		arr.push(start);
		start += ((Math.random()*0.5)-0.25);
	}
	
	return arr;
}

for (var i=0;i<dList.length;i++)
{
	appendDevice(dList[i].name,dList[i].id);
}

graphs = $(".bgChart");

function drawCharts()
{
	for (i=0; i<graphs.length; i+=1)
	{
		$(graphs[i]).sparkline(powerFor(dList[i].id),
		{width: '100%',
		height: '100%',
		type: 'line',
		lineColor: '#bbb',
		fillColor: '#ddd',
		spotColor: false,
		minSpotColor: false,
		maxSpotColor: false,
		highlightSpotColor: false,
		highlightLineColor: '#888',
		chartRangeMin: 0,
		chartRangeMax: 10,
		chartRangeClip: true,
		normalRangeColor: '#c0c0c0',
		drawNormalOnTop: false});
	}
}

drawCharts();

setInterval(drawCharts,1000);
