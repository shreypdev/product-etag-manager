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

$("#logout-btn").click((e) => {
    firebase.auth().signOut();
});

// The below code is for showinf toast notifications
(function(b){b.toast=function(a,h,g,l,k){b("#toast-container").length||(b("body").prepend('<div id="toast-container" aria-live="polite" aria-atomic="true"></div>'),b("#toast-container").append('<div id="toast-wrapper"></div>'));var c="",d="",e="text-muted",f="",m="object"===typeof a?a.title||"":a||"Notice!";h="object"===typeof a?a.subtitle||"":h||"";g="object"===typeof a?a.content||"":g||"";k="object"===typeof a?a.delay||3E3:k||3E3;switch("object"===typeof a?a.type||"":l||"info"){case "info":c="bg-info";
f=e=d="text-white";break;case "success":c="bg-success";f=e=d="text-white";break;case "warning":case "warn":c="bg-warning";f=e=d="text-white";break;case "error":case "danger":c="bg-danger",f=e=d="text-white"}a='<div class="toast" role="alert" aria-live="assertive" aria-atomic="true" data-delay="'+k+'">'+('<div class="toast-header '+c+" "+d+'">')+('<strong class="mr-auto">'+m+"</strong>");a+='<small class="'+e+'">'+h+"</small>";a+='<button type="button" class="ml-2 mb-1 close" data-dismiss="toast" aria-label="Close">';
a+='<span aria-hidden="true" class="'+f+'">&times;</span>';a+="</button>";a+="</div>";""!==g&&(a+='<div class="toast-body">',a+=g,a+="</div>");a+="</div>";b("#toast-wrapper").append(a);b("#toast-wrapper .toast:last").toast("show")}})(jQuery);
//This is the call we make to get toast
// $.toast({
//     title: 'Device added successfully',
//     subtitle: '11 mins ago',
//     content: 'Hey my name is shrey',
//     type: 'success',
//     delay: 5000
// });