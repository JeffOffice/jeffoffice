# Abrir livros protegidos com palavra-passe
<!-- keywords: palavra-passe, protegido, cifrado, cifragem, abrir, bloqueado, palavra-passe errada, cancelar, xlsx, xls, ods, proteção de ficheiros, palavra-passe predefinida, privado -->

Alguns livros são protegidos com uma palavra-passe ao serem guardados: sem ela, nenhum programa consegue ler o que está no ficheiro. Isto é diferente de **Proteger Folha** e **Proteger Livro** (ver [Proteger uma folha ou o livro](protect)), que apenas impedem alterações. Um livro protegido com palavra-passe não pode ser aberto de todo até que a palavra-passe seja escrita. Esta página explica como abrir um destes livros e como criar um.

## Abrir um livro

Abra o ficheiro como abre qualquer outro: prima **{key:Open}**, escolha-o ou arraste-o para a janela. O JeffCrunch verifica que o ficheiro está protegido e pede a palavra-passe antes de ler seja o que for:

1. Escreva a palavra-passe na caixa que diz **Este ficheiro está protegido com palavra-passe.** Os carateres aparecem como pontos.
2. Clique em **Abrir**.

Se a palavra-passe não estiver correta, o JeffCrunch indica-o e volta a pedi-la, quantas vezes quiser. Clique em **Cancelar** para parar: nada é aberto e os livros que já tem abertos mantêm-se como estão. As palavras-passe são exatas: as maiúsculas, os espaços e os acentos contam.

O JeffCrunch não guarda a palavra-passe em lado nenhum. Mantém-na em memória enquanto o livro estiver aberto, para que **Guardar** a possa voltar a proteger, e apaga-a quando fechar o livro. Na próxima vez que abrir o ficheiro, a palavra-passe volta a ser pedida.

## O que pode ser aberto

- **Livros .xlsx e .xlsm** protegidos de uma das duas formas que o programa original utiliza desde 2007: a mais recente (a chave tem 128, 192 ou 256 bits, e o JeffCrunch verifica também que ninguém alterou o ficheiro depois de ser protegido) e a mais antiga (AES com uma chave de 128, 192 ou 256 bits).
- **Livros .xls** protegidos de qualquer uma das três formas desse formato: a cifragem simples dos ficheiros mais antigos, a de 40 bits e a mais forte, que utiliza o serviço de chaves do próprio computador.
- **Folhas de cálculo .ods** protegidas com palavra-passe, em todas as três formas com que o LibreOffice e o OpenOffice as escreveram: a mais recente (o LibreOffice 25 e versões seguintes bloqueiam o ficheiro inteiro com uma chave de 256 bits e geram a chave a partir da palavra-passe com um método propositadamente lento e que utiliza muita memória, pelo que abrir estes ficheiros demora um pouco mais), a de 256 bits das versões anteriores e a mais antiga.

Os ficheiros .xls mais antigos têm limites quanto à própria palavra-passe. A cifragem simples dos ficheiros mais antigos utiliza apenas os primeiros 15 carateres de uma palavra-passe e só pode conter letras, algarismos e sinais do conjunto de carateres da Europa Ocidental (Latin-1): uma palavra-passe com outras letras (como grego ou chinês) nunca é aceite como correta nesse modo. O modo de 40 bits aceita quaisquer letras, mas apenas os primeiros 16 carateres. O JeffCrunch escreve uma cópia .xls com palavra-passe no modo mais forte (o que utiliza o serviço de chaves do computador, com uma chave de 128 bits), porque os outros dois são mais fracos. Esse modo utiliza os primeiros 255 carateres da palavra-passe, que podem ser de quaisquer letras.

Alguns ficheiros estão bloqueados com uma palavra-passe que todos conhecem, para que possam ser abertos por qualquer pessoa, mas não sejam alterados por acidente. O JeffCrunch abre estes ficheiros sem pedir nada.

## Quando um ficheiro não pode ser aberto

- **"A palavra-passe não está correta."** A palavra-passe escrita não é a que foi usada para proteger o ficheiro. Tente de novo, com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma palavra-passe perdida.
- **"Este ficheiro está protegido de uma forma que este programa não consegue abrir."** O ficheiro está bloqueado por outra coisa que não uma palavra-passe (por exemplo, uma chave guardada num certificado), ou numa forma que o JeffCrunch não lê.
- **"A proteção por palavra-passe deste ficheiro exige mais memória ou tempo do que este programa permite, por isso não é aberto."** Um ficheiro criado pelo LibreOffice 25 ou versão posterior define a quantidade de memória e de tempo necessária para verificar a palavra-passe. O JeffCrunch aceita até dez vezes o valor definido pelo próprio LibreOffice (640 MiB de memória), para que um ficheiro feito para sobrecarregar o computador não consiga fazê-lo. Um ficheiro escrito pelo próprio LibreOffice fica bem dentro desse limite.
- **"O ficheiro está danificado ou foi alterado depois de ser protegido."** A palavra-passe estava certa, mas o conteúdo do ficheiro não é o que foi bloqueado: foi cortado, danificado ou alterado por outro programa. Copie-o de novo a partir do original.

## Guardar com palavra-passe

Clique em **Ficheiro** e, depois, em **Proteger com Palavra-passe** (ou **Rever > Proteger com Palavra-passe**), escreva a palavra-passe duas vezes e clique em **Proteger**. O livro é guardado protegido na próxima vez que o guardar: quem abrir o ficheiro tem de escrever a palavra-passe. Para alterar a palavra-passe, escolha o mesmo comando e escreva uma nova; para a retirar, clique em **Remover Palavra-passe** e guarde.

Um livro aberto com palavra-passe mantém-na enquanto estiver aberto, por isso **Guardar** volta a gravar o ficheiro protegido com a mesma palavra-passe. O JeffCrunch só mantém a palavra-passe em memória até fechar o livro, e depois apaga-a. O ficheiro que abriu com palavra-passe não fica desprotegido por acidente: uma cópia guardada como ficheiro .xls fica protegida com a mesma palavra-passe no modo .xls mais forte, e uma cópia guardada como .ods ou .ots fica protegida com ela da forma descrita pela norma OpenDocument, pelo que outros programas que seguem essas normas podem abrir a cópia com a palavra-passe. Uma cópia em texto não pode ser protegida, por isso o JeffCrunch avisa primeiro que a cópia não ficará protegida, e pode cancelar.

Utilize uma palavra-passe longa e guarde-a num local seguro: ninguém, nem nós, consegue abrir o ficheiro sem ela.
