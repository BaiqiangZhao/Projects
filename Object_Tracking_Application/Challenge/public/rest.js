document.addEventListener("DOMContentLoaded",(event) =>{
  
  console.log("Making a RESTful search request to the server!")

  setInterval(function(){
    let result = document.getElementById('result')
    let id = document.getElementById('objects').value
    let theURL='/search/'+id;
    fetch(theURL)
    .then(response=>response.json())
    .then(function(response) {
      result.textContent = response['found'] + "\t" + response['coordinates']
      });
  },2000);


});

// function Search() {
//   let id = document.getElementById('objects').value
//   let theURL='/search/'+id;
//   console.log("Making a RESTful search request to the server!")
//   fetch(theURL)
//   .then(response=>response.json())
//   .then(function(response) {

//     //let img = document.getElementById('image')
//     //img.src = response['img_src']
//     //let text = document.getElementById('text')
//     //text.innerHTML = response['text']
//   });
// }


function Store_Coordinates(){
  let id = document.getElementById('objects').value
  let theURL='/store/'+id;
  console.log("Making a RESTful store request to the server!")
  fetch(theURL)

}
