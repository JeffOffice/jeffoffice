# Pack and Go: levar uma apresentação consigo
<!-- keywords: pack and go, empacotar, enviar, partilhar, outro computador, zip, pasta, tipos de letra, fontes, ligações a ficheiros, imagens ligadas, imagens em falta, copiar, levar consigo, unidade USB, licença, filme, vídeo, som, áudio, apresentação, mostrar -->

Uma apresentação pode reproduzir um filme ou um som a partir de um ficheiro que não está dentro dela. Noutro computador, esse ficheiro não existe, e a apresentação fica com uma falha. O **Pack and Go** guarda a apresentação juntamente com os respetivos ficheiros ligados e tipos de letra, num único ficheiro ZIP ou numa única pasta.

## Como utilizar

1. Clique em **Ficheiro** e, depois, em **Exportar**.
2. Clique em **Pack and Go: Guardar para Outro Computador** para criar um ficheiro ZIP, ou em **Pack and Go: Guardar numa Pasta** para criar uma pasta nova.
3. Escolha onde guardar. Para um ficheiro ZIP, escreva um nome. Para uma pasta, escolha a pasta onde pretende criar a nova.
4. Leia a mensagem que aparece. Indica onde está o pacote, quantos ficheiros ligados e ficheiros de tipos de letra foram incluídos, e tudo o que não pôde ser empacotado.

Um **ficheiro ZIP** é um único ficheiro que contém outros ficheiros, o que facilita o envio. A outra pessoa abre-o para extrair os ficheiros. Uma pasta é mais prática quando pretende copiar tudo para uma unidade.

## O que entra no pacote

- **A própria apresentação**, com o mesmo nome que tem agora. É uma cópia. O seu ficheiro original não é alterado.
- **Filmes e sons ligados.** Um filme ou um som pode estar ligado, o que significa que a apresentação o reproduz a partir do respetivo ficheiro em vez de o conter. O pacote inclui uma cópia de cada um, para que a apresentação funcione da mesma forma no outro computador. Se não colocou o ficheiro dentro da apresentação, é assim que o pode levar consigo.
- **Imagens ligadas.** O pacote inclui uma cópia de cada imagem ligada ao respetivo ficheiro.
- **Tipos de letra.** Um tipo de letra é o aspeto dos carateres. O pacote inclui os ficheiros dos tipos de letra que vêm com o programa e que a apresentação utiliza, numa pasta chamada **fonts**. O Pack and Go nunca empacota um tipo de letra cuja licença indique que não pode ser distribuído. A mensagem indica cada tipo de letra que ficou de fora. Os tipos de letra que instalou no seu computador não são empacotados, por isso a outra pessoa poderá precisar deles.

## Como as ligações mudam

Na cópia, cada ligação que foi empacotada aponta para o ficheiro dentro do pacote, através de um nome curto, como **ligados/foto.png**. As ligações deixam de usar o caminho do seu computador, por isso funcionam onde quer que o pacote seja aberto. Se dois ficheiros tiverem o mesmo nome, ao segundo é acrescentado um número.

Uma ligação que o Pack and Go não consegue seguir mantém-se como estava, e a mensagem indica-a. O Pack and Go não empacota:

- Um ficheiro que não é encontrado.
- Um endereço Web. Esses mantêm-se como estão.
- Um ficheiro existente noutro computador da sua rede.
- Uma ligação que sobe e sai da pasta da apresentação, como **../../notas.png**.
- Um documento, livro de cálculo ou outro objeto que não esteja na pasta da própria apresentação (ou numa pasta dentro dela), mesmo quando a ligação indica o caminho completo. Só são empacotados imagens, filmes e sons, seja de que local for.
- Uma ligação que sai da pasta da apresentação através de um atalho para outra pasta (uma ligação simbólica).
- Uma ligação para a própria apresentação.
- Um ficheiro que não seja uma imagem, um filme, um som ou um documento. Um programa nunca é empacotado.

O Pack and Go não procura mais ligações dentro dos ficheiros ligados, por isso um ficheiro que aponte de volta para este não pode criar um ciclo.

## Limites

Para evitar que um pacote fique demasiado grande, o Pack and Go pergunta primeiro quando existem mais de 200 ficheiros ligados, ou mais de 256 MB de ficheiros ligados. Escolha **Não** e nada é guardado. O Pack and Go nunca empacota mais de 2000 ficheiros nem mais de 1 GB, independentemente da resposta. Os ficheiros que ultrapassem esse limite ficam de fora e são indicados na mensagem.

Se utilizar uma palavra-passe na apresentação, a cópia no pacote tem a mesma palavra-passe.
