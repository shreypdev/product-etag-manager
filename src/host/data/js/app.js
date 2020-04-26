const preventInfo = () => {
	firebase.auth().onAuthStateChanged((user) => {
	    if (!user) {
	        self.location="/";
        }

        const userId = user.uid;

        firebase.database().ref('Data/users-host-mapping/' + userId)
        .on('value', (snap) => {
            let hostSecretKeyFromDB = snap.val();
            
            if(!hostSecretKeyFromDB){
                self.location="/";
            }

            $.post("/confirm-host-secret-key", { hostSecretKey: hostSecretKeyFromDB })
            .done(( resp ) => {
                resp = resp.toLowerCase() == 'true' ? true : false;
                if(!resp) {
                    self.location="/";
                }
            });
        });
	});
};

const forwardToDashboard = () => {
    firebase.auth().onAuthStateChanged((user) => {
        if (!user) {
	        return;
        }

        const userId = user.uid;

        firebase.database().ref('Data/users-host-mapping/' + userId)
        .on('value', (snap) => {
            let hostSecretKeyFromDB = snap.val();
            
            if(!hostSecretKeyFromDB){
                $("#input-error-text").html("Cannot authenticate user on the current host!!");
                $("#email").val("");
                $("#password").val("");
                return;
            }

            $.post("/confirm-host-secret-key", { hostSecretKey: hostSecretKeyFromDB })
            .done(( resp ) => {
                resp = resp == 'true' ? true : false;
                if(!resp) {
                    $("#input-error-text").html("Cannot authenticate user on the current host!!");
                    $("#email").val("");
                    $("#password").val("");
                    return;
                }
                
                self.location="dashboard.html";
            });
        });
    });
};