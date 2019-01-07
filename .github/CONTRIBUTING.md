# Contributing to `cmsgemos`
The following are a set of guidelines to help you in contributing to the `cmsgemos` code base.

## How to start developing
Start by forking the central [`cmsgemos`](https://github.com/cms-gem-daq-project/cmsgemos) repository.
Once you have your fork, then you can check it out to wherever you will be developing.


### Workflow
We have been utilizing a very helpful guideline for our development model outlined here: [`git-flow`](http://nvie.com/posts/a-successful-git-branching-model/)
The basic idea is the following:
* fork from [cms-gem-daq-project/cmsgemos](https://github.com/cms-gem-daq-project/cmsgemos)
* create a branch to develop your particular feature (based off of `develop`, or in some cases, the current `release` branch)
  * `hotfix-` may be created from `master`
  * `bugfix-` may also be created from a given `feature` branch
  * once that feature is completed, create a pull request
* `master` should *always* be stable
  * Do *not* commit directly onto `master` or `develop`, and ensure that your `master` and `develop` are **always** up-to-date with `cms-gem-daq-project` before starting new developments

* Some generally good guidelines (though this post recommends *not* using the `git-flow` model[](https://juliansimioni.com/blog/three-git-commands-you-should-never-use/))
  * *Never* use `git commit -a`
  * *Avoid* `git commit -m` over `git commit -p` or `git commit`, as it will force you to think about your commit message
    * Speaking of... commit messages should be descriptive, not like a novel, but concise and complete.  If they reference an issue or PR, please include that information.
  * *Prefer* `git rebase` over `git pull` (or configure `git pull` to do a `rebase`)
    * You can set this up either in the repo `.git/config` file per repo, or per branch, or globally via `~/.gitconfig`
    * [Golden rebase rules](https://www.atlassian.com/git/tutorials/merging-vs-rebasing#the-golden-rule-of-rebasing)
      * Executive summary: *never* `rebase` a public branch, i.e., a branch you have pushed somewhere, and *especially* not a branch that others may be collaborating with

### Coding Style

cmsgemos loosely follows the [Google style](https://google.github.io/styleguide/cppguide.html#Inline_Functions).

#### Indentation and spaces

* Do not use tabs, use an editor that is smart enough to convert all tabs to spaces
* Use 4 spaces per indentation level for both Python and C++ code
* Don't add trailing whitespaces
* Don't add a whitespace between a function name and the corresponding parenthesis:
  ```cpp
  void foo(int bar);
  // ...
  foo(5);
  ```
* Add a whitespace between a keword and the corresponding parenthesis:
  ```cpp
  if (success) {
      while (true) {
          // ...
      }
  }
  ```

#### Braces and parentheses

Closing braces are indented at the same level as the line they were opened at:
```cpp
void foo()
{
    if (bar()) {
        // ...
    }
}
```

`if` statements without braces are discouraged. They may *only* be used if none of the corresponding `else if` and `else` statements use braces.

Opening braces after class/function definitions go on the subsequent line:
```cpp
class Foo
{
    // ...
};

void bar()
{
    // ...
}
```

In-class function definitions follow a different convention: if possible, they're written on one line (case 1). In case that line would be too long, a line break may be added after the opening brace (case 2). If the function has several expressions, they are written on separate lines (case 3). A trailing semicolon is always added after the closing brace.
```cpp
class Foo
{
    // Case 1
    int bar() { return 0; }; // Notice the second ;
    
    // Case 2
    int longBar() {
        return somethingLong; }; // Notice the second ;
    
    // Case 3
    int veryLongBar() {
        // Two-lines implementation
        int constant = 0;
        return something + constant;
    }; // Notice the ;
};
```

#### Naming conventions
cmsgemos' naming conventions are inspired [from Java](https://en.wikipedia.org/wiki/Naming_convention_(programming)#Java):
* Namespaces names are lowercase and nest easily: `gem::base::utils::exception` is a valid name. All code lives within the `gem` namespace, and every directory at the root of the source tree has its own namespace (the example above is from `gembase`). Header files are organized in a directory structure matching their main namespace: for instance, header files for the above namespace can be found in `include/gem/base/utils/exception`. Header guards use are constructed after the file path and written in UPPER_SNAKE_CASE: for our favorite example, one would use `GEM_BASE_UTILS_EXCEPTION_H`.
* Class, structure and `typedef` names are written in PascalCase. The name of class member variables starts with `m_`, or `p_` for pointers (but this scheme is not used for structures). Accessor functions are named `getVariable()` and `setVariable()`.
* Function (and member function) names are written in camelCase.

### Testing
* You should, at a minimum, test that your code compiles, and if possible, test that it runs without crashing
* When testing the `python` package, you should set up a `virtualenv` and use `pip` to install the `zip` created by:
```
cd gempython
make cleanrpm
make rpm
```
  * If you also need to test this against other `gempython` packages (`gemplotting`, `vfatqc`), you should find the release that is compatible from the releases page of the repository and use `pip` to install them into your `virtualenv`

#### Environment
To be filled in

## Making a pull request
Once you have tested your code, you are ready to make a pull request.  If it references an issue or another pull request, make sure to include that information.

## Continuous Integration
We have set up `travis-ci` to perform some basic CI/CD tasks for `github` pushes, and `gitlab-ci` for the `gitlab` mirror.
The workflow is based on `Docker` images that have been set up to be similar to the 904 lab machines

### Using Labels
#### Issue and Pull Request Labels
There are several labels used to track issues.  Unfortunately, due to the way that `github` is set up, general users are not
able to add these labels.  As such, they are outlined here, and when creating an issue or pull request, should be referenced
in the title so that the maintainers (or a friendly bot) can apply the appropriate label easily.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| ---------- |:------------------------------------------ |:--------------------------------- |:----------- |
| `Type: Bug` | search [`cmsgemos`][search-cmsgemos-repo-label-bug] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-bug] for issues | Issue reports a `bug`, and supplementary information, i.e., how to reproduce, useful debugging info, etc. |
| `Type: Bugfix` | search [`cmsgemos`][search-cmsgemos-repo-label-bugfix] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-bugfix] for issues | Issue reports a `bugfix`, and references the bug issue |
| `Type: Duplicate` | search [`cmsgemos`][search-cmsgemos-repo-label-duplicate] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-duplicate] for issues | Issue will be tagged as `duplicate`, and a reference to the initial issue will be added|
| `Type: Enhancement` | search [`cmsgemos`][search-cmsgemos-repo-label-enhancement] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-enhancement] for issues | Issue reports an `enhancement` |
| `Type: Feature Request` | search [`cmsgemos`][search-cmsgemos-repo-label-feature-request] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-feature-request] for issues | Issue contains a `feature-request` |
| `Type: Maintenance` | search [`cmsgemos`][search-cmsgemos-repo-label-maintenance] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-maintenance] for issues | Issue reports a `maintenance` or `maintenance` request |
| `Type: New Tag` | search [`cmsgemos`][search-cmsgemos-repo-label-new-tag] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-new-tag] for issues | Issue reports a bug, and supplementary information, i.e., how to reproduce, useful debugging info, etc. |
| `Type: Question` | search [`cmsgemos`][search-cmsgemos-repo-label-question] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-question] for issues | Issue raises a question, though it will generally be better to contact on mattermost |
| `Type: Answer` | search [`cmsgemos`][search-cmsgemos-repo-label-answer] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-label-answer] for issues | Issue will answer a previously referenced question|

#### Issue and Pull Request Labels
Maintainers (or a friendly bot) will (hopefully) attach a priority based on the information given in the issue/PR.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| ---------- |:------------------------------------------ |:--------------------------------- |:----------- |
| `Priority: Low` | search [`cmsgemos`][search-cmsgemos-repo-priority-low] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-priority-low] for issues | Priority `low` assigned to issue/PR |
| `Priority: Medium` | search [`cmsgemos`][search-cmsgemos-repo-priority-medium] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-priority-medium] for issues | Priority `medium` assigned to issue/PR |
| `Priority: High` | search [`cmsgemos`][search-cmsgemos-repo-priority-high] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-priority-high] for issues | Priority `high` assigned to issue/PR |
| `Priority: Critical` | search [`cmsgemos`][search-cmsgemos-repo-priority-critical] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-priority-critical] for issues | Priority `critical` assigned to issue/PR |

#### Pull Request Status Labels
Maintainers (or a friendly bot) will (hopefully) properly migrate issues and pull requests through the various stages on their path to resolution.

| Label name | `cms-gem-daq-project/cmsgemos` :mag_right: | `cms-gem-daq-project` :mag_right: | Description |
| ---------- |:------------------------------------------ |:--------------------------------- |:----------- |
| `Status: Blocked` | search [`cmsgemos`][search-cmsgemos-repo-status-blocked] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-blocked] for issues | Issue/PR `blocked`: depends on some other issue/PR (should be referenced) |
| `Status: Pending` | search [`cmsgemos`][search-cmsgemos-repo-status-pending] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-pending] for issues | Issue/PR `pending`: acknowledged, ready to be reviewed |
| `Status: Accepted` | search [`cmsgemos`][search-cmsgemos-repo-status-accepted] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-accepted] for issues | Issue/PR `accepted`: accepted |
| `Status: Completed` | search [`cmsgemos`][search-cmsgemos-repo-status-completed] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-completed] for issues | Issue/PR `completed`: ready for inclusion |
| `Status: Invalid` | search [`cmsgemos`][search-cmsgemos-repo-status-invalid] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-invalid] for issues | Issue/PR `invalid`: invalid, possibly can't reproduce |
| `Status: Wontfix` | search [`cmsgemos`][search-cmsgemos-repo-status-wontfix] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-wontfix] for issues | Issue/PR `wontfix`: won't be included as-is |
| `Status: Wrong Repo` | search [`cmsgemos`][search-cmsgemos-repo-status-wrong-repo] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-wrong-repo] for issues | Issue/PR `wrong-repo`: issue reported in incorrect repository |
| `Status: Help Wanted` | search [`cmsgemos`][search-cmsgemos-repo-status-help-wanted] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-help-wanted] for issues | Issue/PR `help-wanted`: call for someone to take on the task |
| `Status: Revision Needed` | search [`cmsgemos`][search-cmsgemos-repo-status-revision-needed] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-revision-needed] for issues | Issue/PR `revision-needed`: something needs to be changed before proceeding |
| `Status: On Hold` | search [`cmsgemos`][search-cmsgemos-repo-status-on-hold] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-on-hold] for issues | Issue/PR `on-hold`:  being worked on, but either stale, or waiting for inputs |
| `Status: In Progress` | search [`cmsgemos`][search-cmsgemos-repo-status-in-progress] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-in-progress] for issues | Issue/PR `in-progress`:  actively being worked on |
| `Status: Review Needed` | search [`cmsgemos`][search-cmsgemos-repo-status-review-needed] for issues | search [`cms-gem-daq-project`][search-cms-gem-daq-project-status-review-needed] for issues | Issue/PR `review-needed`: ready for inclusion, needs review |

###### Acknowledgements
* Much style and syntax of this was borrowed heavily from the [atom](https://github.com/atom/atom/blob/master/CONTRIBUTING.md) repository

[search-cmsgemos-repo-label-bug]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bug%22
[search-cms-gem-daq-project-label-bug]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bug%22
[search-cmsgemos-repo-label-bugfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bugfix%22
[search-cms-gem-daq-project-label-bugfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Bugfix%22
[search-cmsgemos-repo-label-duplicate]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Duplicate%22
[search-cms-gem-daq-project-label-duplicate]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Duplicate%22
[search-cmsgemos-repo-label-enhancement]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Enhancement%22
[search-cms-gem-daq-project-label-enhancement]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Enhancement%22
[search-cmsgemos-repo-label-feature-request]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Feature+Request%22
[search-cms-gem-daq-project-label-feature-request]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Feature+Request%22
[search-cmsgemos-repo-label-maintenance]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Maintenance%22
[search-cms-gem-daq-project-label-maintenance]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Maintenance%22
[search-cmsgemos-repo-label-question]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Question%22
[search-cms-gem-daq-project-label-question]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Question%22
[search-cmsgemos-repo-label-answer]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+Answer%22
[search-cms-gem-daq-project-label-answer]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+Answer%22
[search-cmsgemos-repo-label-new-tag]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Type%3A+New+Tag%22
[search-cms-gem-daq-project-label-new-tag]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Type%3A+New+Tag%22

[search-cmsgemos-repo-priority-low]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Low%22
[search-cms-gem-daq-project-priority-low]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Low%22
[search-cmsgemos-repo-priority-medium]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Medium%22
[search-cms-gem-daq-project-priority-medium]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Medium%22
[search-cmsgemos-repo-priority-high]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+High%22
[search-cms-gem-daq-project-priority-high]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+High%22
[search-cmsgemos-repo-priority-critical]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Critical%22
[search-cms-gem-daq-project-priority-critical]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Priority%3A+Critical%22

[search-cmsgemos-repo-status-invalid]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Invalid%22
[search-cms-gem-daq-project-status-invalid]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Invalid%22
[search-cmsgemos-repo-status-wontfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wontfix%22
[search-cms-gem-daq-project-status-wontfix]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wontfix%22
[search-cmsgemos-repo-status-accepted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Accepted%22
[search-cms-gem-daq-project-status-accepted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Accepted%22
[search-cmsgemos-repo-status-completed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Completed%22
[search-cms-gem-daq-project-status-completed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Completed%22
[search-cmsgemos-repo-status-pending]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Pending%22
[search-cms-gem-daq-project-status-pending]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Pending%22
[search-cmsgemos-repo-status-blocked]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Blocked%22
[search-cms-gem-daq-project-status-blocked]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Blocked%22
[search-cmsgemos-repo-status-wrong-repo]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wrong+Repo%22
[search-cms-gem-daq-project-status-wrong-repo]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Wrong+Repo%22
[search-cmsgemos-repo-status-help-wanted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Help+Wanted%22
[search-cms-gem-daq-project-status-help-wanted]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Help+Wanted%22
[search-cmsgemos-repo-status-revision-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Revision+Needed%22
[search-cms-gem-daq-project-status-revision-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Revision+Needed%22
[search-cmsgemos-repo-status-review-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+Review+Needed%22
[search-cms-gem-daq-project-status-review-needed]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+Review+Needed%22
[search-cmsgemos-repo-status-on-hold]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+On+Hold%22
[search-cms-gem-daq-project-status-on-hold]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+On+Hold%22
[search-cmsgemos-repo-status-in-progress]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+repo%3Acmsgemos+user%3Acms-gem-daq-project+label%3A%22Status%3A+In+Progress%22
[search-cms-gem-daq-project-status-in-progress]: https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3Acms-gem-daq-project+label%3A%22Status%3A+In+Progress%22
