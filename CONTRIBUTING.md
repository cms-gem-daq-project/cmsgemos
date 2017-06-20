# Contribuging to ```cmsgemos```
The following are a set of guidelines to help you in contributing to the ```cmsgemos``` code base.

## How to start developing
Start by forking the main (```cmsgemos```)[https://github.com/cms-gem-daq-project/cmsgemos] repository.
Once you have your fork, then you can check it out to where you will be developing.


### Workflow
We have been utilizing a very helpful guideline for our development process outlined (here)[http://nvie.com/posts/a-successful-git-branching-model/]
The basic idea is, create a branch to develop your particular feature, or hotfix, once that feature is completed, create a pull request.
Commit messages should be short, but descriptive.  If they reference an issue or PR, please include that information.

### Coding Style
To be filled in

### Testing
To be filled in

## Making a pull request
Once you have tested your code, you are ready to make a pull request.  If it references an issue or another pull request, make sure to include that information.

### Using Labels
#### Issue and Pull Request Labels
There are several labels used to track issues.  Unfortunately, due to the way that github is set up, general users are not
able to add these labels.  As such, they are outlined here, and when creating an issue or pull request, should be referenced
in the title so that the maintainers can apply the appropriate label easily.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| --- | --- | --- | --- |
| `Type: Bug` | [search][search-cmsgemos-repo-label-bug] | [search][search-cms-gem-daq-project-label-bug] | Issue reports a `bug`, and supplementary information, i.e., how to reproduce, useful debugging info, etc. |
| `Type: Bugfixfix` | [search][search-cmsgemos-repo-label-bugfix] | [search][search-cms-gem-daq-project-label-bugfix] | Issue reports a `bugfix`, and references the bug issue |
| `Type: Duplicate` | [search][search-cmsgemos-repo-label-duplicate] | [search][search-cms-gem-daq-project-label-duplicate] | Issue will be tagged as `duplicate`, and a reference to the initial issue will be added|
| `Type: Enhancement` | [search][search-cmsgemos-repo-label-enhancement] | [search][search-cms-gem-daq-project-label-enhancement] | Issue reports an `enhancement` |
| `Type: Feature Request` | [search][search-cmsgemos-repo-label-feature-request] | [search][search-cms-gem-daq-project-label-feature-request] | Issue contains a `feature-request` |
| `Type: Maintenance` | [search][search-cmsgemos-repo-label-maintenance] | [search][search-cms-gem-daq-project-label-maintenance] | Issue reports a `maintenance` or `maintenance` request |
| `Type: New Tag` | [search][search-cmsgemos-repo-label-new-tag] | [search][search-cms-gem-daq-project-label-new-tag] | Issue reports a bug, and supplementary information, i.e., how to reproduce, useful debugging info, etc. |
| `Type: Question` | [search][search-cmsgemos-repo-label-question] | [search][search-cms-gem-daq-project-label-question] | Issue raises a question, though it will generally be better to contact on mattermost |
| `Type: Answer` | [search][search-cmsgemos-repo-label-answer] | [search][search-cms-gem-daq-project-label-answer] | Issue will answer a previously referenced question|

#### Issue and Pull Request Labels
Maintainers will (hopefully) attach a priority based on the information given in the issue/PR.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| --- | --- | --- | --- |
| `Priority: Low` | [search][search-cmsgemos-repo-priority-low] | [search][search-cms-gem-daq-project-priority-low] | Priority `low` assigned to issue/PR |
| `Priority: Medium` | [search][search-cmsgemos-repo-priority-medium] | [search][search-cms-gem-daq-project-priority-medium] | Priority `medium` assigned to issue/PR |
| `Priority: High` | [search][search-cmsgemos-repo-priority-high] | [search][search-cms-gem-daq-project-priority-high] | Priority `high` assigned to issue/PR |
| `Priority: Critical` | [search][search-cmsgemos-repo-priority-critical] | [search][search-cms-gem-daq-project-priority-critical] | Priority `critical` assigned to issue/PR |

#### Pull Request Status Labels
Maintainers will (hopefully) properly migrate issues and pull requests through the various stages on their path to resolution.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| --- | --- | --- | --- |
| `Status: Blocked` | [search][search-cmsgemos-repo-status-blocked] | [search][search-cms-gem-daq-project-status-blocked] | Issue/PR `blocked`: depends on some other issue/PR (should be referenced) |
| `Status: Pending` | [search][search-cmsgemos-repo-status-pending] | [search][search-cms-gem-daq-project-status-pending] | Issue/PR `pending`: acknowledged, ready to be reviewed |
| `Status: Accepted` | [search][search-cmsgemos-repo-status-accepted] | [search][search-cms-gem-daq-project-status-accepted] | Issue/PR `accepted`: accepted |
| `Status: Completed` | [search][search-cmsgemos-repo-status-completed] | [search][search-cms-gem-daq-project-status-completed] | Issue/PR `completed`: ready for inclusion |
| `Status: Invalid` | [search][search-cmsgemos-repo-status-invalid] | [search][search-cms-gem-daq-project-status-invalid] | Issue/PR `invalid`: invalid, possibly can't reproduce |
| `Status: Wontfix` | [search][search-cmsgemos-repo-status-wontfix] | [search][search-cms-gem-daq-project-status-wontfix] | Issue/PR `wontfix`: won't be included as-is |
| `Status: Wrong Repo` | [search][search-cmsgemos-repo-status-wrong-repo] | [search][search-cms-gem-daq-project-status-wrong-repo] | Issue/PR `wrong-repo`: issue reported in incorrect repository |
| `Status: Help Wanted` | [search][search-cmsgemos-repo-status-help-wanted] | [search][search-cms-gem-daq-project-status-help-wanted] | Issue/PR `help-wanted`: call for someone to take on the task |
| `Status: Revision Needed` | [search][search-cmsgemos-repo-status-revision-needed] | [search][search-cms-gem-daq-project-status-revision-needed] | Issue/PR `revision-needed`: something needs to be changed before proceeding |
| `Status: On Hold` | [search][search-cmsgemos-repo-status-on-hold] | [search][search-cms-gem-daq-project-status-on-hold] | Issue/PR `on-hold`:  being worked on, but either stale, or waiting for inputs |
| `Status: In Progress` | [search][search-cmsgemos-repo-status-in-progress] | [search][search-cms-gem-daq-project-status-in-progress] | Issue/PR `in-progress`:  actively being worked on |
| `Status: Review Needed` | [search][search-cmsgemos-repo-status-review-needed] | [search][search-cms-gem-daq-project-status-review-needed] | Issue/PR `review-needed`: ready for inclusion, needs review |


[search-cmsgemos-repo-label-bug]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bug%22
[search-cms-gem-daq-project-label-bug]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bug%22
[search-cmsgemos-repo-label-bugfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bugfix%22
[search-cms-gem-daq-project-label-bugfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bugfix%22
[search-cmsgemos-repo-label-duplicate]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Duplicate%22
[search-cms-gem-daq-project-label-duplicate]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Duplicate%22
[search-cmsgemos-repo-label-enhancement]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Enhancement%22
[search-cms-gem-daq-project-label-enhancement]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Enhancement%22
[search-cmsgemos-repo-label-feature-request]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Feature+Request%22
[search-cms-gem-daq-project-label-feature-request]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Feature+Request%22
[search-cmsgemos-repo-label-maintenance]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Maintenance%22
[search-cms-gem-daq-project-label-maintenance]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Maintenance%22
[search-cmsgemos-repo-label-question]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Question%22
[search-cms-gem-daq-project-label-question]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Question%22
[search-cmsgemos-repo-label-answer]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Answer%22
[search-cms-gem-daq-project-label-answer]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Answer%22
[search-cmsgemos-repo-label-new-tag]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+New+Tag%22
[search-cms-gem-daq-project-label-new-tag]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+New+Tag%22

[search-cmsgemos-repo-priority-low]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Low%22
[search-cms-gem-daq-project-priority-low]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Low%22
[search-cmsgemos-repo-priority-medium]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Medium%22
[search-cms-gem-daq-project-priority-medium]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Medium%22
[search-cmsgemos-repo-priority-high]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+High%22
[search-cms-gem-daq-project-priority-high]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+High%22
[search-cmsgemos-repo-priority-critical]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Critical%22
[search-cms-gem-daq-project-priority-critical]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Critical%22

[search-cmsgemos-repo-status-invalid]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Invalid%22
[search-cms-gem-daq-project-status-invalid]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Invalid%22
[search-cmsgemos-repo-status-wontfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wontfix%22
[search-cms-gem-daq-project-status-wontfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wontfix%22
[search-cmsgemos-repo-status-accepted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Accepted%22
[search-cms-gem-daq-project-status-accepted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Accepted%22
[search-cmsgemos-repo-status-completed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Completed%22
[search-cms-gem-daq-project-status-completed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Completed%22
[search-cmsgemos-repo-status-pending]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Pending%22
[search-cms-gem-daq-project-status-pending]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Pending%22
[search-cmsgemos-repo-status-blocked]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Blocked%22
[search-cms-gem-daq-project-status-blocked]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Blocked%22
[search-cmsgemos-repo-status-wrong-repo]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wrong+Repo%22
[search-cms-gem-daq-project-status-wrong-repo]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wrong+Repo%22
[search-cmsgemos-repo-status-help-wanted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Help+Wanted%22
[search-cms-gem-daq-project-status-help-wanted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Help+Wanted%22
[search-cmsgemos-repo-status-revision-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Revision+Needed%22
[search-cms-gem-daq-project-status-revision-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Revision+Needed%22
[search-cmsgemos-repo-status-review-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Review+Needed%22
[search-cms-gem-daq-project-status-review-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Review+Needed%22
[search-cmsgemos-repo-status-on-hold]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+On+Hold%22
[search-cms-gem-daq-project-status-on-hold]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+On+Hold%22
[search-cmsgemos-repo-status-in-progress]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3cms-gem-daq-project%2cmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+In+Progress%22
[search-cms-gem-daq-project-status-in-progress]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+In+Progress%22
