$("#logout-btn").click((e) => {
    firebase.auth().signOut();
})

$(document).ready(function(){	
	preventInfo(); //Call from app.js
});
