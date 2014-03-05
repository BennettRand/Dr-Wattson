var devices = $("#devices")[0];//document.getElementById("devices");
var details = $("#details")[0];//document.getElementById("details");

var detailsPlot = 0;

$.jqplot.config.defaultHeight = ($(window).height()/2)-50;

var days = ["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"];
var months = ["January","February","March","April","May","June","July","August","September","October","November","December"];

function drawChart(ele, id)
{
	$(ele).sparkline(powerFor(id),
	{width: '100%',
	height: '100%',
	type: 'line',
	lineColor: '#444',
	fillColor: '#888',
	spotColor: false,
	minSpotColor: false,
	maxSpotColor: false,
	highlightSpotColor: false,
	highlightLineColor: '#444',
	chartRangeMin: 0,
	chartRangeMax: 10,
	chartRangeClip: true,
	normalRangeColor: '#c0c0c0',
	drawNormalOnTop: false});
}

function appendDevice(name1, id1, name2, id2)
{
	placeholder = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque semper.";
	
	plug = document.createElement("div");
	plug.setAttribute("class","plug");
	plug.setAttribute("name1",name1);
	plug.setAttribute("id1",id1);
	plug.setAttribute("name2",name2);
	plug.setAttribute("id2",id2);
	devices.appendChild(plug);
	
	
	link1 = document.createElement("a");
	theDiv1 = document.createElement("div");
	header1 = document.createElement("h3");
	text1 = document.createElement("p");
	spark1 = document.createElement("div");
	
	plug.appendChild(link1);
	link1.appendChild(theDiv1);
	theDiv1.appendChild(header1);
	theDiv1.appendChild(spark1);
	
	link1.setAttribute("href", "javascript:void(0)");
	link1.setAttribute("onClick", "getDetailsFor(\""+name1+"\", "+id1.toString()+");");
	
	theDiv1.setAttribute("class","device");
	
	header1.appendChild(document.createTextNode(name1));
	
	text1.appendChild(document.createTextNode(placeholder));
	
	spark1.setAttribute("class", "bgChart");
	drawChart(spark1, id1);
	
	link2 = document.createElement("a");
	theDiv2 = document.createElement("div");
	header2 = document.createElement("h3");
	text2 = document.createElement("p");
	spark2 = document.createElement("div");
	
	plug.appendChild(link2);
	link2.appendChild(theDiv2);
	theDiv2.appendChild(header2);
	theDiv2.appendChild(spark2);
	
	link2.setAttribute("href", "javascript:void(0)");
	link2.setAttribute("onClick", "getDetailsFor(\""+name2+"\", "+id2.toString()+");");
	
	theDiv2.setAttribute("class","device");
	
	header2.appendChild(document.createTextNode(name2));
	
	text2.appendChild(document.createTextNode(placeholder));
	
	spark2.setAttribute("class", "bgChart");
	drawChart(spark2, id1);
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

function dateToStr(d)
{
	timeString = (d.getMonth()+1).toString()+" ";
	timeString += d.getDate().toString()+" ";
	timeString += d.getFullYear().toString()+" ";
	
	var currentHour = d.getHours()%12;
	if (currentHour == 0){currentHour = 12;}
	timeString += (currentHour%12).toString()+":";
	
	if(d.getMinutes() < 10){timeString += "0"+d.getMinutes().toString()+":";}
	else{timeString += d.getMinutes().toString()+":";}
	
	if(d.getSeconds() < 10){timeString += "0"+d.getSeconds().toString()+" ";}
	else{timeString += d.getSeconds().toString()+" ";}
	
	return timeString;
}

function detailsFor(name, id, arr)
{
	closeDetails();
	setTimeout("openDetails(\""+name+"\","+id.toString()+");",250);
	
	$("#chartArea")[0].innerHTML =  '';
	
	detailsPlot = $.jqplot('chartArea', arr,
	{
	title:'Power Data For '+name,
	// Set default options on all series, turn on smoothing.
	seriesDefaults: {
		rendererOptions: {
			smooth: true
		},
		markerOptions: { 
			lineWidth:2,
			size:0
		},
		shadow:false
	},
	// Series options are specified as an array of objects, one object
	// for each series.
	series:[
		{
			// Change our line width and use a diamond shaped marker.
			label:"Voltage",
			yaxis:"yaxis"
		},
		{
			// Don't show a line, just show markers.
			// Make the markers 7 pixels with an 'x' style
			label:"Current",
			yaxis:"y2axis"
		},
		{
			// Use (open) circlular markers.
			label:"Power",
			yaxis:"y3axis"
		}
	],
	legend:{
		show:true,
		location:'nw'
	},
	axesDefaults: {
		useSeriesColor:true,
		rendererOptions: {
			alignTicks: true
		},
		shadow:false
	},
	axes: {
		xaxis: {
			label:'Time',
			padMin:0,
			padMax:0,
			renderer:$.jqplot.DateAxisRenderer,
			tickOptions:{
				formatString:'%#m-%#d\n%I:%M:%S %p'
			}
		},
		yaxis: {
			label:'V'
		},
		y2axis: {
			label:'A',
			min:0
		},
		y3axis: {
			label:'W',
			min:0
		}
	},
	highlighter: {
		show: true,
		sizeAdjust: 7.5,
		tooltipFormatString:'%.2f',
		tooltipAxes: 'y'
	}
	});
	
}

function getDetailsFor(name, id)
{
	/*$.ajax({
		dataType: "json",
		url: "http://"+document.location.host+":8080/testapp/detail",
		data:{id:id},
		success: function(data){
		detailsFor(name, id, data);
		},
		timeout: 10000,
		cache: false,
		crossDomain: true
	});*/
	$("#loading")[0].style.visibility="visible";
	$.getJSON("http://"+document.location.host+":8080/detail",{id:id}, function(data){
		
		detailsFor(name, id, data);
		$("#loading")[0].style.visibility="hidden";
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

function drawSparks()
{
	dList =	[{"name1":"Foo","id1":1,"name2":"Bar","id2":2},
		{"name1":"Baz","id1":3,"name2":"Qux","id2":4},
		{"name1":"Quxom","id1":5,"name2":"Lorem","id2":6},
		{"name1":"Ipsum","id1":7,"name2":"Dolor","id2":8},
		{"name1":"Sit","id1":9,"name2":"Amet","id2":10},
		{"name1":"Foo","id1":1,"name2":"Bar","id2":2},
		{"name1":"Baz","id1":3,"name2":"Qux","id2":4},
		{"name1":"Quxom","id1":5,"name2":"Lorem","id2":6},
		{"name1":"Ipsum","id1":7,"name2":"Dolor","id2":8},
		{"name1":"Sit","id1":9,"name2":"Amet","id2":10}];
	
	$("#loading")[0].style.visibility="visible";
	plugs = $(".plug")
	for (var p = 0; p < plugs.length; p++)
	{
		// console.log(plugs[p]);
		while (plugs[p].firstChild) {
			plugs[p].removeChild(plugs[p].firstChild);
		}
		plugs[p].remove();
	}
	for (var i=0;i<dList.length;i++)
	{
		appendDevice(dList[i].name1,dList[i].id1,dList[i].name2,dList[i].id2);
	}
	$("#loading")[0].style.visibility="hidden";
}

function clockTick()
{
	var d=new Date();
	var timeString = "";
	timeString += days[d.getDay()]+" ";
	timeString += months[d.getMonth()]+" ";
	timeString += d.getDate().toString()+" ";
	// timeString += d.getFullYear().toString()+" ";
	
	var currentHour = d.getHours()%12;
	if (currentHour == 0){currentHour = 12;}
	timeString += (currentHour%12).toString()+":";
	
	if(d.getMinutes() < 10){timeString += "0"+d.getMinutes().toString()+" ";}
	else{timeString += d.getMinutes().toString()+" ";}
	
	// if(d.getSeconds() < 10){timeString += "0"+d.getSeconds().toString()+" ";}
	// else{timeString += d.getSeconds().toString()+" ";}
	
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

drawSparks();
powerCounts();
clockTick();

setInterval(clockTick,1000);
setInterval(powerCounts,1000);
setInterval(drawSparks,10000);
// setInterval(drawCharts,1000);
