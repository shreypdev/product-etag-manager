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
                        firebase.database().ref('Data/' + hostSecretKey + '/users').update(tempJson)
                        .then(() => {
                            console.log("Done!!");
                            self.location="dashboard.html";
                        })
                        .catch(() => {
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
            $("#hostSecretKey").val("");
            return;
        });
    });
});

$("#sign-in").click((e)=>{
    e.preventDefault();

    $("#input-error-text").html("");
    
    const email = $("#email").val();
    const password = $("#password").val();
    const hostSecretKey = $("#hostSecretKey").val();

    if(email == "" || password == "" || hostSecretKey == ""){
        $("#input-error-text").html("Everything on this form is required!!");
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

        firebase.auth().signInWithEmailAndPassword(email, password)
        .then(() => {
            firebase.auth().onAuthStateChanged((user) => {
                if (user) {
                    let userId = user.uid;
                    
                    firebase.database().ref('Data/' + hostSecretKey + '/users/' + userId)
                    .on('value', (snap) => {
                        if(!snap.val()){
                            $("#input-error-text").html("Cannot authenticate with the credentials entered");
                            return;
                        }
                        console.log("Done!!");
                        self.location="dashboard.html";
                    });
                }
            });
        })
        .catch((error) => {
            $("#input-error-text").html(error.message);
            $("#email").val("");
            $("#password").val("");
            $("#hostSecretKey").val("");
            return;
        });
    });
});