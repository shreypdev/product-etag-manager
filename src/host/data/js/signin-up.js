$("#sign-up").click((e)=>{
    e.preventDefault();

    $("#input-error-text").html("");
    
    const name = $("#name").val();
    const email = $("#email").val();
    const password = $("#password").val();
    const confirmPassword = $("#confirmPassword").val();
    const hostSecretKey = $("#hostSecretKey").val();

    if(name == "" || email == "" || password == "" || confirmPassword == "" || hostSecretKey == ""){
        $("#input-error-text").html("Everything on this form is required!!");
        return;
    }
    if(password != confirmPassword){
        $("#input-error-text").html("Confirm password does not match the password!!");
        $("#password").val("");
        $("#confirmPassword").val("");
        return;
    }

    $.post("/confirm-host-secret-key", { hostSecretKey: hostSecretKey })
    .done(( resp ) => {
        resp = resp.toLowerCase() == 'true' ? true : false;
        if(!resp) {
            $("#input-error-text").html("Host secret key verification failed!!");
            $("#hostSecretKey").val("");
            return;
        }

        firebase.auth().createUserWithEmailAndPassword(email, password)
        .then(() => {
            firebase.auth().onAuthStateChanged((user) => {
                if (user) {
                    let userId = user.uid;
                    user.updateProfile({
                        displayName: name
                    })
                    .then(() => {
                        let tempJson = {};
                        tempJson [userId] = name;
                        let dbPromise = firebase.database().ref('Data/' + hostSecretKey + '/users').update(tempJson);
                        dbPromise.then(() => {
                            console.log("Done!!");
                        })
                        dbPromise.catch(() => {
                            $("#input-error-text").html("Something went wrong please try again!!"); 
                            return;
                        });
                    })
                    .catch(() => {
                        $("#input-error-text").html("Something went wrong please try again!!"); 
                        return;      
                    })
                }
            });
        })
        .catch((error) => {
            $("#input-error-text").html(error.message);
            $("#name").val("");
            $("#email").val("")
            $("#password").val("");
            $("#confirmPassword").val("");
            $("#hostSecretKey").val("")
            return;
        });
    });
});