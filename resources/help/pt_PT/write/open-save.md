# Abrir, guardar, imprimir e exportar
<!-- keywords: abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falha, ecrã de ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado -->

Clique em **Ficheiro** no canto superior esquerdo para ver o ecrã de Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar como. Prima **Esc** para voltar ao documento.

## Iniciar um documento novo

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e depois em **Documento em Branco**. Um documento novo abre-se numa janela própria. Se a janela em que está estiver vazia e nunca tiver sido guardada, o documento novo ocupa o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre-se como um documento novo criado a partir dele, pelo que o modelo se mantém tal como está.
- **.docm** e **.dotm** também se abrem. O JeffWrite não mantém as respetivas macros. Avisa-o na primeira vez que guardar um desses documentos e guarda um ficheiro .docx.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite lê-os e guarda o que fizer neles como um novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt, e guarda o que fizer num ficheiro .ott ou .fodt como um novo ficheiro .odt. Consulte [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite lê-o e guarda o que fizer nele como um novo ficheiro .docx. Consulte [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também apresenta os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, mantém-nas. Consulte [Bem-vindo ao JeffWrite](start). Se parte de um ficheiro não puder ser lida de todo, o JeffWrite indica-lhe o que deixou de fora. Nesse caso não guarda por cima desse ficheiro: Guardar pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Guardar um documento

Prima **Ctrl+S** para guardar. Na primeira vez, o JeffWrite pede um nome. **Guardar como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo. Um ficheiro proveniente de um tipo mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro e continua a trabalhar no documento que tinha aberto. O nome e as alterações não guardadas mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Cada vez que guardar, o JeffWrite copia primeiro o ficheiro antigo para outro com o nome "Cópia de segurança de" seguido do nome original, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento que está a escrever não é alterado. Em **Guardar Como**, pode também escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para guardar um ficheiro de modelo onde preferir.

Para iniciar um documento novo a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos estão listados em **Os Meus Modelos**. Clique num deles. Abre-se um documento novo com o texto e os estilos do modelo. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes. Cada forma pede um ficheiro, ou usa o que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, deslocar-se, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, pelo que o ficheiro que abriu não é alterado.
- **Abrir como Cópia** abre um documento novo com o texto do ficheiro. Não tem ficheiro próprio, por isso **Guardar** pergunta onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo quando a janela em que está estiver vazia.
- **Abrir e Reparar** destina-se a um ficheiro .docx danificado, por exemplo um que foi interrompido ao ser copiado. Consulte a secção seguinte.

## Abrir e Reparar

Por vezes um ficheiro .docx fica danificado. Uma cópia pode ter parado a meio, ou um disco pode ter cometido erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite volta a criá-las.

O documento reparado abre-se como cópia, com um nome como "Carta reparada". Uma mensagem indica o que foi mantido até uma interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda uma cópia de reserva do trabalho não guardado a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias da próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e pode guardar cada uma onde quiser.

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, o número de cópias e as páginas, veja cada página na pré-visualização e clique em **Imprimir**. Consulte [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Escolha um nome. O PDF tem o mesmo aspeto em qualquer computador, porque os tipos de letra ficam incorporados nele.

## Ver também

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
