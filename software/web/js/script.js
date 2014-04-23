//  devices = $("#devices")[0];//document.getElementById("devices");
//  details = $("#details")[0];//document.getElementById("details");
 days = ["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"];
 months = ["January","February","March","April","May","June","July","August","September","October","November","December"];

 var detailsPlot;
 
 var segs = 1;
 
function drawChart(ele, id)
{
	$.getJSON("http://"+document.location.host+"/api/spark",{id:id}, function(data){
		$(ele).sparkline(data,
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
		chartRangeClip: true,
		normalRangeColor: '#c0c0c0',
		disableInteraction: true,
		drawNormalOnTop: false});
		latest = data[data.length-1];
		if(latest === undefined)
		{
			return;
		}
		ele.parentElement.children[2].textContent = latest.toString()+"W";
	});
}

function appendDevice(name1, id1, name2, id2)
{
	placeholder = "N/A";
	
	plug = document.createElement("div");
	plug.setAttribute("class","plug");
	plug.setAttribute("name1",name1);
	plug.setAttribute("id1",id1);
	plug.setAttribute("name2",name2);
	plug.setAttribute("id2",id2);
	$("#devices")[0].appendChild(plug);
	
	
	link1 = document.createElement("a");
	theDiv1 = document.createElement("div");
	header1 = document.createElement("h3");
	text1 = document.createElement("h2");
	spark1 = document.createElement("div");
	
	plug.appendChild(link1);
	link1.appendChild(theDiv1);
	theDiv1.appendChild(header1);
	theDiv1.appendChild(spark1);
	theDiv1.appendChild(text1).setAttribute("class","power");
	
	link1.setAttribute("href", "javascript:void(0)");
	link1.setAttribute("onClick", "getDetailsFor(\""+name1+"\", \""+id1.toString()+"\");");
	
	theDiv1.setAttribute("class","device");
	
	header1.appendChild(document.createTextNode(name1));
	
	text1.appendChild(document.createTextNode(placeholder));
	
	spark1.setAttribute("class", "bgChart");
	drawChart(spark1, id1);
	
	link2 = document.createElement("a");
	theDiv2 = document.createElement("div");
	header2 = document.createElement("h3");
	text2 = document.createElement("h2");
	spark2 = document.createElement("div");
	
	plug.appendChild(link2);
	link2.appendChild(theDiv2);
	theDiv2.appendChild(header2);
	theDiv2.appendChild(spark2);
	theDiv2.appendChild(text2).setAttribute("class","power");
	
	link2.setAttribute("href", "javascript:void(0)");
	link2.setAttribute("onClick", "getDetailsFor(\""+name2+"\", \""+id2.toString()+"\");");
	
	theDiv2.setAttribute("class","device");
	
	header2.appendChild(document.createTextNode(name2));
	
	text2.appendChild(document.createTextNode(placeholder));
	
	spark2.setAttribute("class", "bgChart");
	drawChart(spark2, id2);
}

function closeDetails()
{
	$("#devices")[0].style.height = "90%";
	$("#details")[0].style.height = "0";
}

function openDetails(name,id)
{
	$("#devices")[0].style.height = "50%";
	$("#details")[0].style.height = "50%";
	$("#devName")[0].innerHTML = "Device: "+name;
	$("#devMAC")[0].innerHTML = "MAC: "+id.toString();
}

function dateToStr(d)
{
	timeString = (d.getMonth()+1).toString()+" ";
	timeString += d.getDate().toString()+" ";
	timeString += d.getFullYear().toString()+" ";
	
	 currentHour = d.getHours()%12;
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
	setTimeout("openDetails(\""+name+"\",\""+id+"\");",250);
	
	// $("#chartArea")[0].innerHTML =  '';
	while ($("#chartArea")[0].firstChild) {
		$("#chartArea")[0].removeChild($("#chartArea")[0].firstChild);
	}
	
	if (arr[0].length == 0)
	{
		$("#chartArea")[0].innerHTML="<h1>No Data</h1>"
	}
	else
	{
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
			/*{
				// Don't show a line, just show markers.
				// Make the markers 7 pixels with an 'x' style
				label:"Current",
				yaxis:"y2axis"
			},*/
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
					formatString:'%#m-%#d\n%I:%M:%S %p'//'%s'
				}
			},
			yaxis: {
				label:'V',
				min: 100,
				max: 140
			},
			/*y2axis: {
				label:'A',
				min:0
			},*/
			y3axis: {
				label:'W',
				min:0.05
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
	$("#details")[0].dname = name;
	$("#details")[0].did = id;
	// $.getJSON("http://"+document.location.host+":8080/detail",{id:id}, function(data){
	$.getJSON("http://"+document.location.host+"/api/detail",{id:id,segs:segs}, function(data){
		
		detailsFor(name, id, data);
		$("#loading")[0].style.visibility="hidden";
	});
}

function powerFor(id)
{
	 start = 5;
	 arr = [];
	for ( x = 0; x<200; x++)
	{
		arr.push(start);
		start += ((Math.random()*0.5)-0.25);
	}
	
	return arr;
}

function getTotals()
{
	 v = 120+((Math.random()*2)-1);
	 i = ((Math.random()*100));
	 p = v*i;
	
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
	
	// $("#loading")[0].style.visibility="visible";
	$.getJSON("http://"+document.location.host+"/api/devices",{}, function(data)
	// $.getJSON("http://"+document.location.host+":8080/devices",{}, function(data)
	{
		plugs = $(".plug")
		for ( p = 0; p < plugs.length; p++)
		{
			// console.log(plugs[p]);
			while (plugs[p].firstChild) {
				plugs[p].removeChild(plugs[p].firstChild);
			}
			plugs[p].remove();
		}
		for ( i=0;i<data.length;i++)
		{
			appendDevice(data[i].name1,data[i].id1,data[i].name2,data[i].id2);
		}
		// $("#loading")[0].style.visibility="hidden";
	});
}

function clockTick()
{
	 d=new Date();
	 timeString = "";
	timeString += days[d.getDay()]+" ";
	timeString += months[d.getMonth()]+" ";
	timeString += d.getDate().toString()+" ";
	// timeString += d.getFullYear().toString()+" ";
	
	 currentHour = d.getHours()%12;
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
	 tPower = getTotals();
	
	$("#averageVoltage")[0].innerHTML = "Voltage: "+(Math.round(tPower.voltage * 100) / 100).toString()+" V";
	
	$("#totalCurrent")[0].innerHTML = "Current: "+Math.round(tPower.current).toString()+" A";
	
	$("#totalPower")[0].innerHTML = "Power: "+Math.round(tPower.power).toString()+" W";
}

function settingsFor()
{
	window.location = "http://"+document.location.host+"/settings.html?id=" + $('#details')[0].did;
}

// function resizeJqplot()
// {
	height = ($(window).height()/2)-50;
	$.jqplot.config.defaultHeight = height;
	// if(detailsPlot != undefined)
	// {
		// detailsPlot.replot( { clear: true, resetAxes: ['yaxis','y2axis','y3axis'] } );
	// }
	// console.log("resize");
// }

// window.onresize = function(){
	// resizeJqplot();
// };

// $(window).resize(function() {
	// detailsPlot.replot( { resetAxes: true } );
// });

// resizeJqplot()

var refresh;

function t_refresh (timeoutPeriod)
{
	if(refresh != undefined)
	{
		// console.log("Clear previous");
		clearTimeout(refresh);
	}
	refresh = setTimeout(function(){window.location.reload(true);},timeoutPeriod); 
	// console.log("Timeout set");
}

// t_refresh(59000);
// window.onclick=function(){t_refresh(59000);}; //Bandage, not fix
// console.log(window.onclick);

drawSparks();
//powerCounts();
clockTick();

setInterval(clockTick,30000);
// setInterval(powerCounts,10000);
setInterval(drawSparks,30000);
// setInterval(drawCharts,1000);
