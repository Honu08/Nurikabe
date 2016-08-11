//Array to map ID by coordinates in the grid
var INPUT_ID = ["51", "52", "53", "54", "55",
							 	"41", "42", "43", "44", "45",
							 	"31", "32", "33", "34", "35",
							 	"21", "22", "23", "24", "25",
							 	"11", "12", "13", "14", "15",];


//clear button just reload the page :)
$("#clear-page").click(function() {
	window.location = "index.html";
});

//goes here when click solve button
$("#solve").click(function() {
	
	//put every cell in white
	for(var j=0; j<INPUT_ID.length; j++){
		$("#"+(j+1)).css('background-color', '#FFFFFF');				
	}
	
	//get the values of the numbered cell in grid
	var input_values = [];
  for(var i=1; i<=25; i++){
		//condition to only take values from cell that arent in blank ()
		if ($("#"+i).val() !== "") {
			input_values.push("numbered("+INPUT_ID[i-1][0]+","+INPUT_ID[i-1][1]+","+$("#"+i).val()+").");
		}
	}
	
	if(input_values.length > 0){
		console.info(input_values);
		//not empty, call AJAX
		performAjax({//calling AJAX
			"task":"solve",
			"data":input_values},function(data){
			var json = JSON.parse(data);
			console.info(json);
			if(json.length <=0){
				//answer set recieved from server is empty; show alert! no answer set!!
				$.notify({
						title: '<strong>Oops!</strong>',
						message: 'No answer set solution.'
					},{
						type: 'danger',
						animate: {
							enter: 'animated fadeInDown',
							exit: 'animated fadeOutUp'
						}
					});
				
			}else{
				//answer set recieved is not empty
				//show succes alert
				$.notify({
						title: '<strong>Great!</strong>',
						message: 'Puzzle is solved.'
					},{
						type: 'success',
						animate: {
							enter: 'animated rollIn',
							exit: 'animated rollOut'
						}
					});
				//updating the grid with answer set results, mapping cells.
				for(var i=0; i<json.length; i++){
					for(var j=0; j<INPUT_ID.length; j++){
						if(INPUT_ID[j] === json[i]){
							//update blue color in the grid
							$("#"+(j+1)).css('background-color', '#22326F');
						}
					}
					//console.info(json[i]);
				}
			}//end else
		});
	}else{

		console.info("Array is empty!");
		//show alert! grid is empty!!
		$.notify({
			title: '<strong>Heads up!</strong>',
			message: 'Make sure the grid is not empty.'
		},{
			type: 'warning',
			animate: {
				enter: 'animated flipInY',
				exit: 'animated flipOutX'
			}
		});
	}
});

//function to call AJAX by JQUERY
function performAjax(data, callback) {
	$.ajax({
		//directory of the aplication services
		url: "API/main.php",
		type: "POST",//type of seccion 
		data: data,
		success: callback,//function to be call when recieve data fron server
		error: errorFunction
	});
}

//error function if AJAX does not occur
function errorFunction(data) {
	console.info(data.text);
}