var devices = $("#devices")[0];//document.getElementById("devices");
var details = $("#details")[0];//document.getElementById("details");

var detailsPlot = 0;

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
	$("#devName")[0].innerHTML = "Device: "+name;
	$("#devMAC")[0].innerHTML = "MAC: "+id.toString();
}

function getDetailsFor(id)
{
	var vStart = 120;
	var iStart = 0;
	
	var vArr = [];
	var pArr = [];
	var iArr = [];
	for (var x = 0; x<200; x++)
	{
		vArr.push([x,vStart]);
		iArr.push([x,iStart]);
		pArr.push([x,(iStart*vStart)]);
		
		vStart += ((Math.random()*0.5)-0.25);
		iStart += ((Math.random()*0.5)-0.25);
		if (vStart < 0){start = 0;}
		if (iStart < 0){start = 0;}
	}
	
	return [vArr,iArr,pArr];
}

function detailsFor(name, id)
{
	closeDetails();
	setTimeout("openDetails(\""+name+"\","+id.toString()+");",250);
	
	$("#chartArea")[0].innerHTML =  '';
	
	detailsPlot = $.jqplot('chartArea', getDetailsFor(id),
	{
	title:'Power Data For '+name,
	// Set default options on all series, turn on smoothing.
	seriesDefaults: {
		rendererOptions: {
			smooth: true
		}
	},
	// Series options are specified as an array of objects, one object
	// for each series.
	series:[
		{
			// Change our line width and use a diamond shaped marker.
			lineWidth:2,
			markerOptions: { style:'dimaond' }
		},
		{
			// Don't show a line, just show markers.
			// Make the markers 7 pixels with an 'x' style
			showLine:false,
			markerOptions: { size: 7, style:"x" }
		},
		{
			// Use (open) circlular markers.
			markerOptions: { style:"circle" }
		}
	]
	});
	
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

function getTotals()
{
	var v = 120+((Math.random()*2)-1);
	var i = ((Math.random()*100));
	var p = v*i;
	
	return {voltage:v,power:p,current:i};
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

var days = ["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"];
var months = ["January","February","March","April","May","June","July","August","September","October","November","December"];

function clockTick()
{
	var d=new Date();
	var timeString = days[d.getDay()]+" ";
	timeString += months[d.getMonth()]+" ";
	timeString += d.getDate().toString()+" ";
	timeString += d.getFullYear().toString()+" ";
	
	var currentHour = d.getHours()%12;
	if (currentHour == 0){currentHour = 12;}
	timeString += (currentHour%12).toString()+":";
	
	if(d.getMinutes() < 10){timeString += "0"+d.getMinutes().toString()+":";}
	else{timeString += d.getMinutes().toString()+":";}
	
	if(d.getSeconds() < 10){timeString += "0"+d.getSeconds().toString()+" ";}
	else{timeString += d.getSeconds().toString()+" ";}
	
	if(d.getHours() < 12){timeString += "AM";}
	else{timeString += "PM";}
	
	$("#dateTime")[0].innerHTML = timeString;
}

function powerCounts()
{
	var tPower = getTotals();
	
	$("#averageVoltage")[0].innerHTML = "Voltage: "+(Math.round(tPower.voltage * 100) / 100).toString()+" V";
	
	$("#totalCurrent")[0].innerHTML = "Current: "+Math.round(tPower.current).toString()+" A";
	
	$("#totalPower")[0].innerHTML = "Power: "+Math.round(tPower.power).toString()+" W";
}

powerCounts();
drawCharts();
clockTick();

setInterval(clockTick,1000);
setInterval(powerCounts,1000);
// setInterval(drawCharts,1000);
