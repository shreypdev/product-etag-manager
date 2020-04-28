let tagInfo = {
    tag_name_orginal: '',
    tag_name_given: '',
    tag_ip: ''
};

let currentHost = 'abc-123';

$("#searchNewTags-btn").click((e) => {
    e.preventDefault();

    $("#tag-search-notification").html("Searching for new tags...");
    $("#populate-new-tags").html("");
    $("#searchNewTags-btn").prop('disabled', true);

    $.get("/find-tags", (data) => {
        let tags = data.devices;
        let html = ``;
        let newTagsCount = 0;


        firebase.database().ref('Data/hosts/' + currentHost + '/tags/registered')
        .once('value', (snap) => {
            let registeredTags = snap.val();

            if(!registeredTags){
                registeredTags = [];
            }

            tags.forEach((tag) => {
                let tagName = tag.tagName;
                let tagID = tagName.substr(tagName.length - 4, 4);

                if(!registeredTags.includes(tagID)){
                    html += `
                        <li class="list-group-item d-flex justify-content-between align-items-center">
                            Tag - ${tagID}
                            <a class="btn btn-primary btn-circle btn-sm add-tag" ip="${tag.ip}" name="${tagName}">
                                <i class="fas fa-plus"></i>
                            </a>
                        </li>
                    `;
                    newTagsCount++;
                }
            });

            let notification_msg = (newTagsCount) ? "" : "No new tags were found on the local network, please try again!!";
            $("#tag-search-notification").html(notification_msg);
            $("#populate-new-tags").html(html);
            $("#searchNewTags-btn").prop('disabled', false);

            //Defining add-tag function for currently showed tags       
            $(".add-tag").click((e) => {
                tagInfo.tag_name_orginal = $(e.currentTarget).attr('name');
                tagInfo.tag_ip = $(e.currentTarget).attr('ip');
                $("#addTagGetMoreInfoModal").modal();
            });
        });
    });
});

$("#add-tag-confirmation").click((e) => {
    $("#device-name-error-text").html('');
    $("#add-tag-confirmation").prop('disabled', true);
    
    let tag_name_given = $("#tagName").val();
    if(tag_name_given === ''){
        $("#device-name-error-text").html('Device name cannot be empty!!');
        $("#add-tag-confirmation").prop('disabled', false);
        return;
    }
    tagInfo.tag_name_given = tag_name_given;

    firebase.database().ref('Data/hosts/' + currentHost + '/tags/detail').push(tagInfo)
    .then(() => {
        firebase.database().ref('Data/hosts/' + currentHost + '/tags/registered')
        .once('value', (snap) => {
            let registeredTags = snap.val();
            if(!registeredTags){
                registeredTags = [];
            }
            registeredTags.push(tagInfo.tag_name_orginal.substr(tagInfo.tag_name_orginal.length-4, 4));
            registeredTags.sort();
            firebase.database().ref('Data/hosts/' + currentHost + '/tags/registered').update(registeredTags)
            .then(() => {
                //  Enable add button again and close modal.
                $("#addTagGetMoreInfoModal").modal('hide');
                $("#add-tag-confirmation").prop('disabled', false);

                // Remove Added tag from Add tag section
                $("a[name='"+ tagInfo.tag_name_orginal +"']").parent().remove();
                // Check if Add new tag section is empty or still has devices
                if(!$("#populate-new-tags").children().length){
                    $("#tag-search-notification").html("Click on search to find tags <br>on the local network");
                }

                //  Show notification
                $.toast({
                    title: 'Device added successfully',
                    subtitle: '0 mins ago',
                    content: 'New devices named '+ tagInfo.tag_name_given +' is added to the '+ currentHost +' host.',
                    type: 'success',
                    delay: 5000
                });
            })
            .catch((error) => {
                console.log(error.message);
                $("#add-tag-confirmation").prop('disabled', false);
                return;
            });
        });
    })
    .catch((error) => {
        console.log(error.message);
        $("#add-tag-confirmation").prop('disabled', false);
        return;
    })
});

const getTagsDetails = () => {
    return new Promise((resolve, reject) => {
        firebase.database().ref('Data/hosts/' + currentHost + '/tags/detail')
        .once('value', (snap) => {
            let tags = snap.val();
            if(!tags){
                tags = {};
            }
            let tagsKey = Object.keys(tags);
            tags.keys = tagsKey;
            resolve(tags);
        });
    });
};

const populateRemoveTagsSection = async (firstTimeCall) => {
    let tags = await getTagsDetails();
    let html = ``;

    tags.keys.forEach((key) => {
        html += `
            <tr>
                <td>
                <div class="list-group-item d-flex justify-content-between align-items-center">
                    ${tags[key].tag_name_given}
                    <a class="btn btn-danger btn-circle btn-sm remove-tag text-white" ip="${tags[key].ip}" key="${key}">
                        <i class="fas fa-minus"></i>
                    </a>
                </div>
                </td>
            </tr>
        `;
    });

    $("#populate-remove-tags").html(html);

    if(firstTimeCall){
        $('#populate-remove-tags-table').DataTable({
            "lengthChange": false,
            "info": false,
            "dom": 'ftp'
        });
    }
};

const liveUpdateFromDB_Tags = () => {
    firebase.database().ref('Data/hosts/' + currentHost + '/tags')
    .on('value', (snap) => {
        populateRemoveTagsSection(false);
    });
};

$(document).ready(() => {
    preventInfo(); //Call from app.js
    populateRemoveTagsSection(true);
    liveUpdateFromDB_Tags();
});
