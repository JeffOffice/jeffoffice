# Abrir, guardar, imprimir e exportar
<!-- keywords: correio eletrónico, eml, marcadores, marcadores pdf, shift+f5, última edição, onde parou, modelos incorporados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falhanço, ecrã de ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, somente leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado -->

Clique em **Ficheiro** no canto superior esquerdo para ver o ecrã Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar Como. Prima **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e depois em **Documento em Branco**. Um novo documento abre numa janela própria. Se a janela em que está estiver vazia e nunca tiver sido guardada, o novo documento toma o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, pelo que o modelo se mantém tal como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um ficheiro. O JeffWrite nunca os executa, e mantém-nos como estavam quando guarda como .docm ou .dotm. Se guardar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa-o.
- **.txt** é texto simples, apenas as palavras. Ao abri-lo, o JeffWrite lê-o de imediato se for ASCII ou Unicode simples. Se não for, o JeffWrite pergunta em que codificação o deve ler e mostra as primeiras linhas, para que possa ver se as letras estão corretas. Uma **codificação** é a forma como um ficheiro guarda as letras, como UTF-8 ou Windows 1252. Ao guardar como .txt, escolhe a codificação e os finais de linha, e o ficheiro guarda apenas as palavras: sem tipos de letra, imagens ou linhas de tabela.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite lê-os, e guarda o que faz com eles como um novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt, e guarda o que faz com um ficheiro .ott ou .fodt como um novo ficheiro .odt. Consulte [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite lê-o e guarda-o. Consulte [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também lista os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, mantém-nas. Consulte [Bem-vindo ao JeffWrite](start). Se parte de um ficheiro não puder ser lida de todo, o JeffWrite diz-lhe o que deixou de fora. Nesse caso, não guarda por cima desse ficheiro: Guardar pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Guardar um documento

Prima **Ctrl+S** para guardar. Na primeira vez, o JeffWrite pede um nome. **Guardar Como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo, **Documento com Macros (*.docm)** para manter as macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Formato RTF (*.rtf)** para um ficheiro que outros programas possam ler. Um ficheiro que veio de um tipo mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro, e continua a trabalhar no documento que tinha aberto. O nome e as alterações não guardadas mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffWrite copia primeiro o ficheiro antigo para outro com o nome "Cópia de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, salvo se disser Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento que está a escrever não é alterado. Em **Guardar Como**, também pode escolher **Modelo (*.dotx)**, ou **Modelo de Texto OpenDocument (*.ott)**, para guardar um ficheiro de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos estão listados em **Os Meus Modelos**. Clique num deles. Abre-se um novo documento com o texto e os estilos do modelo. Para remover um modelo da lista, clique nele com o botão direito e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes. Cada forma pede um ficheiro, ou usa o que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, deslocar, procurar e copiar, mas não pode alterar nada. A barra de título indica "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre um novo documento com o texto do ficheiro. Não tem ficheiro próprio, por isso **Guardar** pergunta onde o deve colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .docx danificado, por exemplo um que foi cortado ao ser copiado. Consulte abaixo.

## Abrir e Reparar

Por vezes, um ficheiro .docx fica danificado. Uma cópia parou a meio, ou um disco cometeu erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto onde parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite volta a criá-las.

O documento reparado abre como cópia, com um nome como "Carta reparada". Uma mensagem indica o que foi mantido até uma interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio, e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda uma cópia de reserva do trabalho não guardado a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias da próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e pode guardar cada uma onde quiser.

## Iniciar a partir de um modelo

**Ficheiro > Novo** mostra o **Documento em Branco** e um conjunto de modelos que vêm com o JeffWrite, agrupados por tipo: uma carta, um relatório, um resumo, um manuscrito e um boletim informativo. Cada imagem é a primeira página do documento que obtém. Um **modelo** é um documento já preparado, com os seus estilos, margens e texto de exemplo. Clique num deles e substitua o texto de exemplo pelo seu. Os modelos que guardou por si próprio estão listados em **Os Meus Modelos**.

## Continuar de onde parou

O JeffWrite lembra-se de onde estava o cursor quando guardou ou fechou um ficheiro. Da próxima vez que abrir o ficheiro, o cursor estará nesse ponto. Prima **Shift+F5** para ir aos locais que editou por último. Cada toque leva ao anterior: os últimos três locais, e depois volta ao primeiro.

## Guardar como .odt

Um ficheiro .odt não consegue conter tudo o que um ficheiro .docx consegue. Ao guardar como .odt, o JeffWrite lista primeiro o que se perderia, e permite guardar um ficheiro .docx em vez disso. Consulte [Informações](info).

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, as cópias e as páginas, veja cada página na pré-visualização e clique em **Imprimir**. Consulte [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Escolha as páginas a incluir: todas, a página atual, ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois escolha um nome. O PDF tem o mesmo aspeto em todos os computadores, porque os tipos de letra ficam incorporados nele. O título, o autor, o assunto e as palavras-chave do documento são transferidos para o PDF. Cada título torna-se um **marcador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique num marcador para ir até à respetiva página.

## Criar um correio eletrónico

Clique em **Ficheiro > Exportar > Criar Correio Eletrónico** para criar um ficheiro de mensagem (.eml). Nada é enviado. Escolha o que vai na mensagem: o documento, o documento como PDF, ou as páginas apresentadas como imagens na mensagem (as primeiras 30 páginas). Abra o ficheiro no seu programa de correio eletrónico, indique a quem se destina e envie-o.

## Consulte também

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
