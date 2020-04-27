let tagInfo = {
    tag_name_orginal: '',
    tag_name_given: '',
    tag_ip: ''
};

$("#searchNewTags-btn").click((e) => {
    e.preventDefault();

    $("#tag-search-notification").html("Searching for new tags...");
    $("#populate-new-tags").html("");
    $("#searchNewTags-btn").prop('disabled', true);

    $.get("/find-tags", (data) => {
        let tags = data.devices;
        let html = ``;

        tags.forEach((tag) => {
            let tagName = tag.tagName;
            let tagID = tagName.substr(tagName.length - 4, 4);
            html += `
                <li class="list-group-item d-flex justify-content-between align-items-center">
                    Tag - ${tagID}
                    <a class="btn btn-primary btn-circle btn-sm add-tag" ip="${tag.ip}" name="${tagName}">
                        <i class="fas fa-plus"></i>
                    </a>
                </li>
            `;
        });

        let notification_msg = (tags.length) ? "" : "No tags found on the local network";
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

$("#add-tag-confirmation").click((e) => {
    console.log("Done");
});

$(document).ready(() => {	
	preventInfo(); //Call from app.js
});
